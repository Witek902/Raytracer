#include "PCH.h"
#include "Scene.h"
#include "Light/BackgroundLight.h"
#include "Object/SceneObject_Light.h"
#include "Object/SceneObject_Decal.h"
#include "Rendering/ShadingData.h"
#include "BVH/BVHBuilder.h"
#include "Material/Material.h"
#include "Utils/Profiler.h"

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
    mTraceableObjects.Clear();
    mDecals.Clear();
    mLights.Clear();
    mGlobalLights.Clear();
    for (const auto& object : mAllObjects)
    {
        if (object->GetType() == ISceneObject::Type::Light)
        {
            const LightSceneObject* lightObject = static_cast<const LightSceneObject*>(object.get());
            mLights.PushBack(lightObject);

            const ILight& light = lightObject->GetLight();

            if (light.GetFlags() & ILight::Flag_IsFinite)
            {
                mTraceableObjects.PushBack(static_cast<const ITraceableSceneObject*>(object.get()));
            }
            else if (!(light.GetFlags() & ILight::Flag_IsFinite))
            {
                mGlobalLights.PushBack(lightObject);
            }
        }
        else if (object->GetType() == ISceneObject::Type::Shape)
        {
            mTraceableObjects.PushBack(static_cast<const ITraceableSceneObject*>(object.get()));
        }
        else if (object->GetType() == ISceneObject::Type::Decal)
        {
            const DecalSceneObject* decalObject = static_cast<const DecalSceneObject*>(object.get());
            mDecals.PushBack(decalObject);
        }
    }

    // build BVH for traceable objects
    {
        DynArray<Box> boxes;
        for (const ISceneObject* obj : mTraceableObjects)
        {
            boxes.PushBack(obj->GetBoundingBox());
        }

        BVHBuilder::Indices newOrder;
        BVHBuilder bvhBuilder(mTraceableObjectsBVH);
        if (!bvhBuilder.Build(boxes.Data(), mTraceableObjects.Size(), BvhBuildingParams(), newOrder))
        {
            return false;
        }

        DynArray<const ITraceableSceneObject*> newObjectsArray;
        newObjectsArray.Reserve(mTraceableObjects.Size());
        for (uint32 i = 0; i < mTraceableObjects.Size(); ++i)
        {
            uint32 sourceIndex = newOrder[i];
            newObjectsArray.PushBack(mTraceableObjects[sourceIndex]);
        }
        mTraceableObjects = std::move(newObjectsArray);
    }

    // build BVH for decals
    {
        DynArray<Box> boxes;
        for (const DecalSceneObject* decal : mDecals)
        {
            boxes.PushBack(decal->GetBoundingBox());
        }

        BvhBuildingParams params;
        params.heuristics = BvhBuildingParams::Heuristics::Volume;

        BVHBuilder::Indices newOrder;
        BVHBuilder bvhBuilder(mDecalsBVH);
        if (!bvhBuilder.Build(boxes.Data(), boxes.Size(), params, newOrder))
        {
            return false;
        }

        DynArray<const DecalSceneObject*> newObjectsArray;
        newObjectsArray.Reserve(mDecals.Size());
        for (uint32 i = 0; i < mDecals.Size(); ++i)
        {
            uint32 sourceIndex = newOrder[i];
            newObjectsArray.PushBack(mDecals[sourceIndex]);
        }
        mDecals = std::move(newObjectsArray);
    }

    return true;
}

void Scene::Traverse_Object(const SingleTraversalContext& context, const uint32 objectID) const
{
    const ITraceableSceneObject* object = mTraceableObjects[objectID];

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

bool Scene::Traverse_Object_Shadow(const SingleTraversalContext& context, const uint32 objectID) const
{
    const ITraceableSceneObject* object = mTraceableObjects[objectID];

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

void Scene::Traverse_Leaf(const SingleTraversalContext& context, const uint32 objectID, const BVH::Node& node) const
{
    RT_UNUSED(objectID);

    const uint32 numLeaves = node.numLeaves;
    const uint32 firstChild = node.childIndex;

    for (uint32 i = 0; i < numLeaves; ++i)
    {
        Traverse_Object(context, firstChild + i);
    }
}

bool Scene::Traverse_Leaf_Shadow(const SingleTraversalContext& context, const BVH::Node& node) const
{
    const uint32 numLeaves = node.numLeaves;
    const uint32 firstChild = node.childIndex;

    for (uint32 i = 0; i < numLeaves; ++i)
    {
        if (Traverse_Object_Shadow(context, firstChild + i))
        {
            return true;
        }
    }

    return false;
}

void Scene::Traverse_Leaf(const PacketTraversalContext& context, const uint32 objectID, const BVH::Node& node, uint32 numActiveGroups) const
{
    RT_UNUSED(objectID);

    for (uint32 i = 0; i < node.numLeaves; ++i)
    {
        const uint32 objectIndex = node.childIndex + i;
        const ITraceableSceneObject* object = mTraceableObjects[objectIndex];
        const Matrix4 invTransform = object->GetInverseTransform(context.context.time);

        // transform ray to local-space
        for (uint32 j = 0; j < numActiveGroups; ++j)
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
    RT_SCOPED_TIMER(Scene_Traverse);

    context.context.localCounters.Reset();

    const uint32 numObjects = mTraceableObjects.Size();

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
    const uint32 numObjects = mTraceableObjects.Size();

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
    const uint32 numObjects = mTraceableObjects.Size();

    const uint32 numRayGroups = context.ray.GetNumGroups();
    for (uint32 i = 0; i < numRayGroups; ++i)
    {
        context.ray.groups[i].maxDistances = VECTOR8_MAX;
        context.context.activeGroupsIndices[i] = (uint16)i;
    }

    for (uint32 i = 0; i < context.ray.numRays; ++i)
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
        const ISceneObject* object = mTraceableObjects.Front();
        const Matrix4 invTransform = object->GetInverseTransform(context.context.time);

        for (uint32 j = 0; j < numRayGroups; ++j)
        {
            RayGroup& rayGroup = context.ray.groups[context.context.activeGroupsIndices[j]];
            rayGroup.rays[1].origin = invTransform.TransformPoint(rayGroup.rays[0].origin);
            rayGroup.rays[1].dir = invTransform.TransformVector(rayGroup.rays[0].dir);
            rayGroup.rays[1].invDir = Vector3x8::FastReciprocal(rayGroup.rays[1].dir);
        }

        mTraceableObjects.Front()->Traverse(context, 0, numRayGroups);
    }
    else // full BVH traversal
    {
        GenericTraverse<Scene, 0>(context, 0, this, numRayGroups);
    }
}

void Scene::EvaluateIntersection(const Ray& ray, const HitPoint& hitPoint, const float time, IntersectionData& outData) const
{
    RT_SCOPED_TIMER(Scene_EvaluateIntersection);

    RT_ASSERT(hitPoint.distance < FLT_MAX);

    const ITraceableSceneObject* object = mTraceableObjects[hitPoint.objectId];

    const Matrix4 transform = object->GetTransform(time);
    const Matrix4 invTransform = transform.FastInverseNoScale();

    const Vector4 worldPosition = ray.GetAtDistance(hitPoint.distance);
    outData.frame[3] = invTransform.TransformPoint(worldPosition);

    // calculate normal, tangent, tex coord, etc. from intersection data
    object->EvaluateIntersection(hitPoint, outData);
    RT_ASSERT(outData.texCoord.IsValid());

    Vector4 localSpaceTangent = outData.frame[0];
    Vector4 localSpaceNormal = outData.frame[2];
    Vector4 localSpaceBitangent = Vector4::Cross3(localSpaceTangent, localSpaceNormal);

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

void Scene::EvaluateShadingData(ShadingData& shadingData, RenderingContext& context) const
{
    RT_ASSERT(shadingData.intersection.material != nullptr);
    shadingData.intersection.material->EvaluateShadingData(context.wavelength, shadingData);

    EvaluateDecals(shadingData, context);
}

void Scene::EvaluateDecals(ShadingData& shadingData, RenderingContext& context) const
{
    if (mDecals.Empty())
    {
        return; // no decals
    }

    const uint32 maxOverlappingDecals = 8;
    uint32 numOverlappingDecals = 0;
    const DecalSceneObject* decals[maxOverlappingDecals];

    // collect overlapping decals

    const Vector4& point = shadingData.intersection.frame.GetTranslation();

    // all nodes
    const BVH::Node* __restrict nodes = mDecalsBVH.GetNodes();

    // "nodes to visit" stack
    uint32 stackSize = 0;
    const BVH::Node* __restrict nodesStack[BVH::MaxDepth];

    // BVH traversal
    for (const BVH::Node* __restrict currentNode = nodes;;)
    {
        if (currentNode->IsLeaf())
        {
            const uint32 numLeaves = currentNode->numLeaves;
            const uint32 firstChild = currentNode->childIndex;

            for (uint32 i = 0; i < numLeaves; ++i)
            {
                RT_ASSERT(numOverlappingDecals < maxOverlappingDecals);
                decals[numOverlappingDecals++] = mDecals[firstChild + i];
            }
        }
        else
        {
            const BVH::Node* __restrict childA = nodes + currentNode->childIndex;
            const BVH::Node* __restrict childB = childA + 1;

            bool hitA = childA->GetBox().Intersects(point);
            bool hitB = childB->GetBox().Intersects(point);

            if (hitA && hitB)
            {
                currentNode = childA;
                nodesStack[stackSize++] = childB;
                continue;
            }
            if (hitA)
            {
                currentNode = childA;
                continue;
            }
            if (hitB)
            {
                currentNode = childB;
                continue;
            }
        }

        if (stackSize == 0)
        {
            break;
        }

        // pop a node
        currentNode = nodesStack[--stackSize];
    }

    if (numOverlappingDecals > 0u)
    {
        struct DecalSorter
        {
            RT_FORCE_INLINE bool operator() (const DecalSceneObject* a, const DecalSceneObject* b) const
            {
                return a->order > b->order;
            }
        };
        std::sort(decals, decals + numOverlappingDecals, DecalSorter());

        // apply decals
        for (uint32 i = 0; i < numOverlappingDecals; ++i)
        {
            decals[i]->Apply(shadingData, context);
        }
    }
}

} // namespace rt
