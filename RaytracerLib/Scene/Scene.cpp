#include "PCH.h"
#include "Scene.h"
#include "Rendering/Settings.h"

#include "Mesh/Mesh.h"


namespace rt {

using namespace math;

Scene::Scene()
{
}

MeshInstanceID Scene::CreateMeshInstance(const MeshInstance& data)
{
    MeshInstanceID id = static_cast<MeshInstanceID>(mMeshInstances.size());
    mMeshInstances.push_back(data);
    return id;
}

void Scene::DestroyMeshInstance(MeshInstanceID id)
{
    // TODO
    (void)id;
}

void Scene::UpdateMeshInstance(MeshInstanceID id, const MeshInstance& data)
{
    (void)id;
    (void)data;
}

void Scene::Traverse_Packet(const RayPacket& packet, RenderingContext& context, RayPacketIntersectionData& outIntersectionData) const
{
    (void)context;

    // TODO BVH for instances
    for (size_t i = 0; i < mMeshInstances.size(); ++i)
    {
        const Uint32 instanceIndex = (Uint32)i;
        const MeshInstance& meshInstance = mMeshInstances[i];

        // TODO transform rays in the packet

        meshInstance.mMesh->RayTrace_Packet(packet, outIntersectionData, instanceIndex);
    }
}

Vector4 Scene::TraceRay_Single(const Ray& ray, RenderingContext& context) const
{
    Ray currentRay = ray;

    Vector4 resultColor;
    Vector4 throughput = VECTOR_ONE;

    for (Uint32 i = 0; i < context.params.maxRayDepth; ++i)
    {
        const MeshInstance* bestMeshInstance = nullptr;

        LocalCounters localCounters;
        RayIntersectionData intersectionData;
        for (const MeshInstance& meshInstance : mMeshInstances)
        {
            // TODO transform ray

            const float previousDistance = intersectionData.distance;
            meshInstance.mMesh->RayTrace_Single(currentRay, intersectionData, localCounters);

            if (intersectionData.distance != previousDistance)
            {
                // we hit this mesh instance
                bestMeshInstance = &meshInstance;
            }
        }

        // TODO move to separate function
        if (context.params.renderingMode == RenderingMode::Depth)
        {
            if (!bestMeshInstance)
            {
                resultColor = Vector4(1.0f, 0.0f, 1.0f);
            }
            else
            {
                const float logDepth = std::max<float>(0.0f, (log2f(intersectionData.distance) + 5.0f) / 10.0f);
                resultColor = Vector4::Splat(logDepth);
            }
            break;
        }
        else if (context.params.renderingMode == RenderingMode::RayBoxIntersection)
        {
            const float num = static_cast<float>(localCounters.numRayBoxTests);
            resultColor = Vector4(num * 0.02f, num * 0.005f, num * 0.001f);
            break;
        }
        else if (context.params.renderingMode == RenderingMode::RayBoxIntersectionPassed)
        {
            const float num = static_cast<float>(localCounters.numPassedRayBoxTests);
            resultColor = Vector4(num * 0.02f, num * 0.005f, num * 0.001f);
            break;
        }
        else if (context.params.renderingMode == RenderingMode::RayTriIntersection)
        {
            const float num = static_cast<float>(localCounters.numRayTriangleTests);
            resultColor = Vector4(num * 0.02f, num * 0.005f, num * 0.001f);
            break;
        }
        else if (context.params.renderingMode == RenderingMode::RayTriIntersectionPassed)
        {
            const float num = static_cast<float>(localCounters.numPassedRayTriangleTests);
            resultColor = Vector4(num * 0.02f, num * 0.005f, num * 0.001f);
            break;
        }

        // ray missed - return background color
        if (!bestMeshInstance)
        {
            resultColor += throughput * mEnvironment.backgroundColor;
            break;
        }


        ShadingData shadingData;
        bestMeshInstance->mMesh->EvaluateShadingData_Single(currentRay, intersectionData, shadingData);

        // TODO move to separate function
        if (context.params.renderingMode == RenderingMode::Normals)
        {
            resultColor = 0.5f * shadingData.normal + math::VECTOR_HALVES;
            break;
        }
        else if (context.params.renderingMode == RenderingMode::TexCoords)
        {
            resultColor = Vector4(fmodf(shadingData.texCoord[0], 1.0f), fmodf(shadingData.texCoord[1], 1.0f));
            break;
        }
        else if (context.params.renderingMode == RenderingMode::BaseColor)
        {
            resultColor = shadingData.material->GetBaseColor(shadingData.texCoord);
            break;
        }

        // accumulate emission color
        resultColor += throughput * shadingData.material->emissionColor;

        const Vector4 baseColor = shadingData.material->GetBaseColor(shadingData.texCoord);
        if (baseColor.IsZero())
        {
            break;
        }

        // accumulate attenuation
        throughput *= baseColor;

        // Russian roulette algorithm
        {
            const float threshold = Max(throughput[0], Max(throughput[1], throughput[2]));
            if (context.randomGenerator.GetFloat() > threshold)
                break;

            throughput /= threshold;
        }

        // generate secondary ray
        const Vector4 incomingDir = currentRay.dir;
        math::Vector4 factor;
        if (!shadingData.material->GenerateSecondaryRay(incomingDir, shadingData, context.randomGenerator, currentRay, factor))
        {
            break;
        }
        throughput *= factor;
    }

    return resultColor;
}

} // namespace rt
