#include "PCH.h"
#include "Scene.h"
#include "Light/BackgroundLight.h"
#include "Object/SceneObject_Light.h"
#include "Rendering/ShadingData.h"
#include "BVH/BVHBuilder.h"
#include "Material/Material.h"

#include "Traversal/Traversal_Single.h"
#include "Traversal/Traversal_Packet.h"

namespace rt {

using namespace math;

Scene::Scene() = default;

Scene::~Scene() = default;

Scene::Scene(Scene&&) = default;

Scene& Scene::operator = (Scene&&) = default;

void Scene::AddObject(SceneObjectPtr object)
{
    if (object->GetType() == ISceneObject::Type::Light)
    {
        mLights.PushBack(static_cast<const LightSceneObject*>(object.get()));
    }

    mAllObjects.PushBack(std::move(object));
}

bool Scene::BuildBVH()
{
    // determine objects to be added to the BVH
    mObjectsInBvh.Clear();
    mLights.Clear();
    mGlobalLights.Clear();
    for (const auto& object : mAllObjects)
    {
        if (object->GetType() == ISceneObject::Type::Light)
        {
            const LightSceneObject* lightObject = static_cast<const LightSceneObject*>(object.get());
            mLights.PushBack(lightObject);

            const ILight& light = lightObject->GetLight();

            auto a = light.GetFlags() & ILight::Flag_IsFinite;
            if (a)
            {
                mObjectsInBvh.PushBack(object.get());
            }
            else if (!(light.GetFlags() & ILight::Flag_IsFinite))
            {
                mGlobalLights.PushBack(lightObject);
            }
        }
        else
        {
            mObjectsInBvh.PushBack(object.get());
        }
    }

    // collect boxes for BVH construction
    DynArray<Box> boxes;
    for (const ISceneObject* obj : mObjectsInBvh)
    {
        boxes.PushBack(obj->GetBoundingBox());
    }

    BVHBuilder::BuildingParams params;
    params.maxLeafNodeSize = 2;

    BVHBuilder::Indices newOrder;
    BVHBuilder bvhBuilder(mBVH);
    if (!bvhBuilder.Build(boxes.Data(), mObjectsInBvh.Size(), params, newOrder))
    {
        return false;
    }

    DynArray<const ISceneObject*> newObjectsArray;
    newObjectsArray.Reserve(mObjectsInBvh.Size());
    for (Uint32 i = 0; i < mObjectsInBvh.Size(); ++i)
    {
        Uint32 sourceIndex = newOrder[i];
        newObjectsArray.PushBack(mObjectsInBvh[sourceIndex]);
    }
    mObjectsInBvh = std::move(newObjectsArray);

    return true;
}

void Scene::Traverse_Object(const SingleTraversalContext& context, const Uint32 objectID) const
{
    const ISceneObject* object = mObjectsInBvh[objectID];

    const Matrix4 invTransform = object->GetInverseTransform(context.context.time);

    // transform ray to local-space
    const Ray transformedRay = invTransform.TransformRay_Unsafe(context.ray);

    const SingleTraversalContext objectContext =
    {
        transformedRay,
        context.hitPoint,
        context.context
    };

    object->Traverse(objectContext, objectID);
}

bool Scene::Traverse_Object_Shadow(const SingleTraversalContext& context, const Uint32 objectID) const
{
    const ISceneObject* object = mObjectsInBvh[objectID];

    const Matrix4 invTransform = object->GetInverseTransform(context.context.time);

    // transform ray to local-space
    Ray transformedRay = invTransform.TransformRay_Unsafe(context.ray);
    transformedRay.originDivDir = transformedRay.origin * transformedRay.invDir;

    const SingleTraversalContext objectContext =
    {
        transformedRay,
        context.hitPoint,
        context.context
    };

    return object->Traverse_Shadow(objectContext);
}

void Scene::Traverse_Leaf(const SingleTraversalContext& context, const Uint32 objectID, const BVH::Node& node) const
{
    RT_UNUSED(objectID);

    for (Uint32 i = 0; i < node.numLeaves; ++i)
    {
        const Uint32 objectIndex = node.childIndex + i;
        Traverse_Object(context, objectIndex);
    }
}

bool Scene::Traverse_Leaf_Shadow(const SingleTraversalContext& context, const BVH::Node& node) const
{
    for (Uint32 i = 0; i < node.numLeaves; ++i)
    {
        const Uint32 objectIndex = node.childIndex + i;
        if (Traverse_Object_Shadow(context, objectIndex))
        {
            return true;
        }
    }

    return false;
}

void Scene::Traverse_Leaf(const PacketTraversalContext& context, const Uint32 objectID, const BVH::Node& node, Uint32 numActiveGroups) const
{
    RT_UNUSED(objectID);

    for (Uint32 i = 0; i < node.numLeaves; ++i)
    {
        const Uint32 objectIndex = node.childIndex + i;
        const ISceneObject* object = mObjectsInBvh[objectIndex];
        const Matrix4 invTransform = object->GetInverseTransform(context.context.time);

        // transform ray to local-space
        for (Uint32 j = 0; j < numActiveGroups; ++j)
        {
            RayGroup& rayGroup = context.ray.groups[context.context.activeGroupsIndices[j]];
            rayGroup.rays[1].origin = invTransform.TransformPoint(rayGroup.rays[0].origin);
            rayGroup.rays[1].dir = invTransform.TransformVector(rayGroup.rays[0].dir);
            rayGroup.rays[1].invDir = Vector3x8::FastReciprocal(rayGroup.rays[1].dir);
        }

        object->Traverse(context, objectIndex, numActiveGroups);
    }
}

void Scene::Traverse(const SingleTraversalContext& context) const
{
    context.context.localCounters.Reset();

    const Uint32 numObjects = mObjectsInBvh.Size();

    if (numObjects == 0)
    {
        // scene is empty
    }
    else if (numObjects == 1)
    {
        // bypass BVH
        Traverse_Object(context, 0);
    }
    else
    {
        // full BVH traversal
        GenericTraverse(context, 0, this);
    }

    context.context.counters.Append(context.context.localCounters);
}

bool Scene::Traverse_Shadow(const SingleTraversalContext& context) const
{
    const Uint32 numObjects = mObjectsInBvh.Size();

    if (numObjects == 0) // scene is empty
    {
        return false;
    }
    else if (numObjects == 1) // bypass BVH
    {
        return Traverse_Object_Shadow(context, 0);
    }
    else // full BVH traversal
    {
        return GenericTraverse_Shadow(context, this);
    }
}

void Scene::Traverse(const PacketTraversalContext& context) const
{
    const Uint32 numObjects = mObjectsInBvh.Size();

    const Uint32 numRayGroups = context.ray.GetNumGroups();
    for (Uint32 i = 0; i < numRayGroups; ++i)
    {
        context.ray.groups[i].maxDistances = VECTOR8_MAX;
        context.context.activeGroupsIndices[i] = (Uint16)i;
    }

    for (Uint32 i = 0; i < context.ray.numRays; ++i)
    {
        context.context.hitPoints[i].distance = FLT_MAX;
        context.context.hitPoints[i].objectId = UINT32_MAX;
    }

    if (numObjects == 0) // scene is empty
    {
        return;
    }
    else if (numObjects == 1) // bypass BVH
    {
        const ISceneObject* object = mObjectsInBvh.Front();
        const Matrix4 invTransform = object->GetInverseTransform(context.context.time);

        for (Uint32 j = 0; j < numRayGroups; ++j)
        {
            RayGroup& rayGroup = context.ray.groups[context.context.activeGroupsIndices[j]];
            rayGroup.rays[1].origin = invTransform.TransformPoint(rayGroup.rays[0].origin);
            rayGroup.rays[1].dir = invTransform.TransformVector(rayGroup.rays[0].dir);
            rayGroup.rays[1].invDir = Vector3x8::FastReciprocal(rayGroup.rays[1].dir);
        }

        mObjectsInBvh.Front()->Traverse(context, 0, numRayGroups);
    }
    else // full BVH traversal
    {
        GenericTraverse<Scene, 0>(context, 0, this, numRayGroups);
    }
}

RT_FORCE_NOINLINE
void Scene::EvaluateIntersection(const Ray& ray, const HitPoint& hitPoint, const float time, IntersectionData& outData) const
{
    RT_ASSERT(hitPoint.distance < FLT_MAX);

    const ISceneObject* object = mObjectsInBvh[hitPoint.objectId];

    const Matrix4 transform = object->GetTransform(time);
    const Matrix4 invTransform = transform.FastInverseNoScale();

    const Vector4 worldPosition = ray.GetAtDistance(hitPoint.distance);
    outData.frame[3] = invTransform.TransformPoint(worldPosition);

    // calculate normal, tangent, tex coord, etc. from intersection data
    object->EvaluateIntersection(hitPoint, outData);
    RT_ASSERT(outData.texCoord.IsValid());

    Vector4 localSpaceTangent = outData.frame[0];
    Vector4 localSpaceBitangent = outData.frame[1];
    Vector4 localSpaceNormal = outData.frame[2];

    // apply normal mapping
    if (outData.material && outData.material->normalMap)
    {
        const Vector4 localNormal = outData.material->GetNormalVector(outData.texCoord);

        // transform normal vector
        Vector4 newNormal = localSpaceTangent * localNormal.x;
        newNormal = Vector4::MulAndAdd(localSpaceBitangent, localNormal.y, newNormal);
        newNormal = Vector4::MulAndAdd(localSpaceNormal, localNormal.z, newNormal);
        localSpaceNormal = newNormal.FastNormalized3();
    }

    // orthogonalize tangent vector (required due to normal/tangent vectors interpolation and normal mapping)
    // TODO this can be skipped if the tangent vector is the same for every point on the triangle (flat shading)
    // and normal mapping is disabled
    localSpaceTangent = Vector4::Orthogonalize(localSpaceTangent, localSpaceNormal).Normalized3();

    // transform shading data from local space to world space
    outData.frame[2] = transform.TransformVector(localSpaceNormal);
    outData.frame[0] = transform.TransformVector(localSpaceTangent);
    outData.frame[1] = Vector4::Cross3(outData.frame[0], outData.frame[2]);
    outData.frame[3] = worldPosition;

    // make sure the frame is orthonormal
    {
        // validate length
        RT_ASSERT(Abs(1.0f - outData.frame[0].SqrLength3()) < 0.001f);
        RT_ASSERT(Abs(1.0f - outData.frame[2].SqrLength3()) < 0.001f);

        // validate perpendicularity
        RT_ASSERT(Vector4::Dot3(outData.frame[1], outData.frame[0]) < 0.001f);
        //RT_ASSERT(Vector4::Dot3(outData.frame[1], outData.frame[2]) < 0.001f);
        //RT_ASSERT(Vector4::Dot3(outData.frame[2], outData.frame[0]) < 0.001f);

        // validate headness
        //const Vector4 computedNormal = Vector4::Cross3(outData.frame[0], outData.frame[1]);
        //RT_ASSERT((computedNormal - outData.frame[2]).SqrLength3() < 0.001f);
    }
}

} // namespace rt
