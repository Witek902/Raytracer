#include "PCH.h"
#include "Scene.h"
#include "Rendering/Context.h"

#include "Mesh/Mesh.h"
#include "Material/Material.h"
#include "Math/Transcendental.h"
#include "Utils/Bitmap.h"
#include "BVH/BVHBuilder.h"

namespace rt {

using namespace math;

Scene::Scene()
{
}

void Scene::SetEnvironment(const SceneEnvironment& env)
{
    mEnvironment = env;
}

void Scene::AddObject(SceneObjectPtr object)
{
    mObjects.push_back(std::move(object));
}

bool Scene::BuildBVH()
{
    std::vector<Box, AlignmentAllocator<Box>> boxes;

    for (const auto& obj : mObjects)
    {
        boxes.push_back(obj->GetBoundingBox());
    }

    BVHBuilder::BuildingParams params;
    params.maxLeafNodeSize = 2;

    BVHBuilder::Indices newOrder;
    BVHBuilder bvhBuilder(mBVH);
    if (!bvhBuilder.Build(boxes.data(), (Uint32)mObjects.size(), params, newOrder))
    {
        return false;
    }

    std::vector<SceneObjectPtr> newObjectsArray;
    newObjectsArray.reserve(mObjects.size());
    for (size_t i = 0; i < mObjects.size(); ++i)
    {
        Uint32 sourceIndex = newOrder[i];
        newObjectsArray.push_back(std::move(mObjects[sourceIndex]));
    }

    mObjects = std::move(newObjectsArray);

    return true;
}

// TODO this has nothing to do with a scene
RayColor Scene::HandleSpecialRenderingMode(RenderingContext& context, const HitPoint& hitPoint, const ShadingData& shadingData)
{
    if (context.params.renderingMode == RenderingMode::Depth)
    {
        const float logDepth = std::max<float>(0.0f, (log2f(hitPoint.distance) + 5.0f) / 10.0f);
        return Vector4::Splat(logDepth);
    }
    else if (context.params.renderingMode == RenderingMode::RayBoxIntersection)
    {
        const float num = static_cast<float>(context.localCounters.numRayBoxTests);
        return Vector4(num * 0.02f, num * 0.005f, num * 0.001f);
    }
    else if (context.params.renderingMode == RenderingMode::RayBoxIntersectionPassed)
    {
        const float num = static_cast<float>(context.localCounters.numPassedRayBoxTests);
        return Vector4(num * 0.02f, num * 0.005f, num * 0.001f);
    }
    else if (context.params.renderingMode == RenderingMode::RayTriIntersection)
    {
        const float num = static_cast<float>(context.localCounters.numRayTriangleTests);
        return Vector4(num * 0.02f, num * 0.005f, num * 0.001f);
    }
    else if (context.params.renderingMode == RenderingMode::RayTriIntersectionPassed)
    {
        const float num = static_cast<float>(context.localCounters.numPassedRayTriangleTests);
        return Vector4(num * 0.02f, num * 0.005f, num * 0.001f);
    }
    else if (context.params.renderingMode == RenderingMode::Normals)
    {
        return 0.5f * shadingData.normal + math::VECTOR_HALVES;
    }
    else if (context.params.renderingMode == RenderingMode::Tangents)
    {
        return 0.5f * shadingData.tangent + math::VECTOR_HALVES;
    }
    else if (context.params.renderingMode == RenderingMode::Position)
    {
        return 0.5f * shadingData.position + math::VECTOR_HALVES;
    }
    else if (context.params.renderingMode == RenderingMode::TexCoords)
    {
        return Vector4(fmodf(shadingData.texCoord[0], 1.0f), fmodf(shadingData.texCoord[1], 1.0f));
    }
    else if (context.params.renderingMode == RenderingMode::BaseColor)
    {
        return shadingData.material->GetBaseColor(shadingData.texCoord);
    }

    return RayColor();
}

void Scene::Traverse_Single(const math::Ray& ray, HitPoint& outHitPoint, RenderingContext& context) const
{
    // TODO BVH traversal
    for (size_t i = 0; i < mObjects.size(); ++i)
    {
        const ISceneObject* object = mObjects[i].get();

        const auto invTransform = object->GetInverseTransform(context.time);

        // transform ray to local-space
        math::Ray transformedRay;
        transformedRay.origin = invTransform.TransformPoint(ray.origin);
        transformedRay.dir = invTransform.TransformVector(ray.dir);
        transformedRay.invDir = Vector4::FastReciprocal(transformedRay.dir);

        const float previousDistance = outHitPoint.distance;
        object->Traverse_Single(transformedRay, outHitPoint);

        if (outHitPoint.distance != previousDistance)
        {
            // we hit this mesh instance
            outHitPoint.objectId = (Uint32)i;
        }
    }
}

void Scene::Traverse_Simd8(const math::Ray_Simd8& ray, HitPoint_Simd8& outHitPoint, RenderingContext& context) const
{
    // TODO BVH traversal
    for (size_t i = 0; i < mObjects.size(); ++i)
    {
        const ISceneObject* object = mObjects[i].get();

        const auto invTransform = object->GetInverseTransform(context.time);

        // transform ray to local-space
        math::Ray_Simd8 transformedRay;
        transformedRay.origin = invTransform.TransformPoint(ray.origin);
        transformedRay.dir = invTransform.TransformVector(ray.dir);
        transformedRay.invDir = Vector3_Simd8::FastReciprocal(transformedRay.dir);

        // TODO
        (void)outHitPoint;
        /*
        const float previousDistance = outHitPoint.distance;
        object->Traverse_Single(transformedRay, outHitPoint);

        if (outHitPoint.distance != previousDistance)
        {
            // we hit this mesh instance
            outHitPoint.objectId = (Uint32)i;
        }
        */
    }
}

void Scene::ExtractShadingData(const math::Ray& ray, const HitPoint& hitPoint, ShadingData& outShadingData) const
{
    const float time = 0.0f;

    outShadingData.position = ray.origin + ray.dir * hitPoint.distance;

    // calculate normal, tangent, tex coord, etc. from intersection data
    const ISceneObject* object = mObjects[hitPoint.objectId].get();
    const auto invTransform = object->GetInverseTransform(time); // HACK
    object->EvaluateShadingData_Single(invTransform, hitPoint, outShadingData);

    // transform shading data from local space to world space
    const auto transform = object->GetTransform(time);
    outShadingData.tangent = transform.TransformVector(outShadingData.tangent);
    outShadingData.bitangent = transform.TransformVector(outShadingData.bitangent);
    outShadingData.normal = transform.TransformVector(outShadingData.normal);
}

RayColor Scene::TraceRay_Single(const Ray& ray, RenderingContext& context) const
{
    const float epsilon = 0.001f;

    const bool regularRenderingMode = context.params.renderingMode == RenderingMode::Regular;

    Ray currentRay = ray;

    RayColor resultColor;
    RayColor throughput = RayColor::One();


    ShadingData shadingData;

    for (Uint32 depth = 0; depth < context.params.maxRayDepth; ++depth)
    {
        HitPoint hitPoint;
        Traverse_Single(currentRay, hitPoint, context);

        // ray missed - return background color
        if (hitPoint.objectId == -1)
        {
            if (regularRenderingMode)
            {
                resultColor += throughput * GetBackgroundColor(currentRay);
            }
            else
            {
                resultColor = Vector4(1.0f, 0.0f, 1.0f);;
            }
            break;
        }

        ExtractShadingData(currentRay, hitPoint, shadingData);

        if (!regularRenderingMode)
        {
            resultColor = HandleSpecialRenderingMode(context, hitPoint, shadingData);
            break;
        }

        // accumulate emission color
        resultColor += throughput * shadingData.material->emissionColor;

        // Russian roulette algorithm
        if (depth >= context.params.minRussianRouletteDepth)
        {
            const float threshold = throughput.values.HorizontalMax()[0];
            if (context.randomGenerator.GetFloat() > threshold)
                break;

            throughput *= 1.0f / Max(FLT_EPSILON, threshold);
        }

        const Vector4 outgoingDirWorldSpace = -currentRay.dir;
        Vector4 incomingDirWorldSpace;
        throughput *= shadingData.material->Shade(outgoingDirWorldSpace, incomingDirWorldSpace, shadingData, context.randomGenerator);

        // ray is not visible anymore
        if (throughput.values.IsZero())
        {
            break;
        }

        // generate secondary ray
        currentRay = math::Ray(shadingData.position, incomingDirWorldSpace);
    }

    return resultColor;
}

void Scene::TraceRay_Simd8(const math::Ray_Simd8& simdRay, RenderingContext& context, RayColor* outColors) const
{
    const bool regularRenderingMode = context.params.renderingMode == RenderingMode::Regular;

    HitPoint_Simd8 hitPoint;
    ShadingData shadingData;
    Traverse_Simd8(simdRay, hitPoint, context);

    for (Uint32 i = 0; i < 8; ++i)
    {
        RayColor resultColor;

        // ray missed - return background color
        if (hitPoint.objectId.u[i] == UINT32_MAX)
        {
            const Vector4 backgroundColor = regularRenderingMode ? mEnvironment.backgroundColor : Vector4(1.0f, 0.0f, 1.0f);
            //resultColor += throughput * mEnvironment.backgroundColor;
            resultColor = backgroundColor;
        }
        else
        {
            /*
            // TODO remove
            Ray ray;
            ray.dir = Vector4(simdRay.dir.x[i], simdRay.dir.y[i], simdRay.dir.z[i]);
            ray.origin = Vector4(simdRay.origin.x[i], simdRay.origin.y[i], simdRay.origin.z[i]);
            //ray.invDir = Vector4(simdRay.dir.x[i], simdRay.dir.y[i], simdRay.dir.z[i]);

            const MeshInstance& meshInstance = mMeshInstances[hitPoint.objectId.u[i]];
            meshInstance.mMesh->EvaluateShadingData_Single(ray, hitPoint.Get(i), shadingData);
            //resultColor = shadingData.material->GetBaseColor(shadingData.texCoord);

            resultColor = shadingData.normal * 0.5f + VECTOR_HALVES;
            */
        }

        // TODO push secondary rays to output stream

        outColors[i] = resultColor;
    }
}

/*
void Scene::Traverse_Packet(const RayPacket& packet, RenderingContext& context, HitPoint_Packet& outHitPoints) const
{
    (void)context;

    // clear hit-points
    for (Uint32 i = 0; i < packet.numRays; ++i)
    {
        outHitPoints[i] = HitPoint();
    }

    // TODO BVH for instances
    for (size_t i = 0; i < mMeshInstances.size(); ++i)
    {
        const Uint32 instanceIndex = (Uint32)i;
        const MeshInstance& meshInstance = mMeshInstances[i];

        // TODO transform rays in the packet

        meshInstance.mMesh->RayTrace_Packet(packet, outHitPoints, instanceIndex);
    }
}

void Scene::ShadePacket(const RayPacket& packet, const HitPoint_Packet& hitPoints, RenderingContext& context, Bitmap& renderTarget) const
{
    const bool regularRenderingMode = context.params.renderingMode == RenderingMode::Regular;

    ShadingData shadingData;

    for (Uint32 i = 0; i < packet.numRays; ++i)
    {
        const HitPoint& hitPoint = hitPoints[i];
        const Ray& ray = packet.rays[i];

        RayColor resultColor;

        // ray missed - return background color
        if (hitPoint.objectId == UINT32_MAX)
        {
            const RayColor backgroundColor = regularRenderingMode ? GetBackgroundColor(ray) : Vector4(1.0f, 0.0f, 1.0f);
            //resultColor += throughput * mEnvironment.backgroundColor;
            resultColor = backgroundColor;
        }
        else
        {
            const ISceneObject* object = mObjects[hitPoint.objectId].get();
            object->EvaluateShadingData_Single(ray, hitPoint, shadingData);
            //resultColor = shadingData.material->GetBaseColor(shadingData.texCoord);

            resultColor = shadingData.normal * 0.5f + VECTOR_HALVES;
        }

        // TODO push secondary rays to output stream

        // TODO this should be performed using dedicated Bitmap method in one batch
        const ImageLocationInfo& imageLocation = packet.imageLocations[i];
        const Vector4 currentColor = renderTarget.GetPixel(imageLocation.x, imageLocation.y);
        const Vector4 rayWeight = packet.weights[i];
        renderTarget.SetPixel(imageLocation.x, imageLocation.y, currentColor + rayWeight * resultColor.values);
    }
}
*/

RayColor Scene::GetBackgroundColor(const math::Ray& ray) const
{
    RayColor color = mEnvironment.backgroundColor;

    // sample environment map
    if (mEnvironment.texture)
    {
        const Float theta = ACos(ray.dir[1]);
        const Float phi = atan2f(ray.dir[2], ray.dir[0]);
        const Vector4 coords(phi / (2.0f * RT_PI) + 0.5f, theta / RT_PI);

        SamplerDesc sampler;
        color *= mEnvironment.texture->Sample(coords, sampler);
    }

    return color;
}

} // namespace rt
