#include "PCH.h"
#include "SceneObject_Plane.h"
#include "Rendering/ShadingData.h"
#include "Rendering/Context.h"
#include "Traversal/TraversalContext.h"

namespace rt {

using namespace math;

PlaneSceneObject::PlaneSceneObject(const Float2 size, const Float2 texScale)
    : mSize(size)
    , mTextureScale(texScale)
{ }

Box PlaneSceneObject::GetBoundingBox() const
{
    const Box localBox(Vector4(-mSize.x, 0.0f, -mSize.y, 0.0f), Vector4(mSize.x, 0.0f, mSize.y, 0.0f));
    return mTransform.TransformBox(localBox); // TODO motion blur
}

bool PlaneSceneObject::Traverse_Single_Internal(const SingleTraversalContext& context, float& outDist) const
{
    if (Abs(context.ray.dir.y) > RT_EPSILON)
    {
        const float t = -context.ray.origin.y * context.ray.invDir.y;

        if (t > 0.0f && t < context.hitPoint.distance)
        {
            const Vector4 pos = context.ray.GetAtDistance(t);

            const Vector4 texCoords = pos.Swizzle<0,2,0,0>();
            if ((Vector4::Abs(texCoords) < Vector4(mSize)).GetMask() == 0x3)
            {
                outDist = t;
                return true;
            }
        }
    }

    return false;
}

void PlaneSceneObject::Traverse_Single(const SingleTraversalContext& context, const Uint32 objectID) const
{
    float t;
    if (Traverse_Single_Internal(context, t))
    {
        context.hitPoint.distance = t;
        context.hitPoint.objectId = objectID;
        context.hitPoint.subObjectId = 0;
    }
}

bool PlaneSceneObject::Traverse_Shadow_Single(const SingleTraversalContext& context) const
{
    return Traverse_Single_Internal(context, context.hitPoint.distance);
}

void PlaneSceneObject::Traverse_Packet(const PacketTraversalContext& context, const Uint32 objectID, const Uint32 numActiveGroups) const
{
    for (Uint32 i = 0; i < numActiveGroups; ++i)
    {
        RayGroup& rayGroup = context.ray.groups[context.context.activeGroupsIndices[i]];

        const Vector8 t = -rayGroup.rays[1].origin.y * rayGroup.rays[1].invDir.y;
        VectorBool8 mask = (t > Vector8::Zero()) & (t < rayGroup.maxDistances);

        if (mask.None())
        {
            continue;
        }

        const Vector8 x = Vector8::MulAndAdd(rayGroup.rays[1].dir.x, t, rayGroup.rays[1].origin.x);
        const Vector8 z = Vector8::MulAndAdd(rayGroup.rays[1].dir.z, t, rayGroup.rays[1].origin.z);
        mask = mask & (Vector8::Abs(x) < Vector8(mSize.x)) & (Vector8::Abs(z) < Vector8(mSize.y));

        context.StoreIntersection(rayGroup, t, mask, objectID);
    }
}

void PlaneSceneObject::EvaluateShadingData_Single(const HitPoint& hitPoint, ShadingData& outShadingData) const
{
    RT_UNUSED(hitPoint);

    outShadingData.material = mDefaultMaterial.get();
    outShadingData.texCoord = (outShadingData.frame.GetTranslation().Swizzle<0,2,0,0>() & Vector4::MakeMask<1,1,0,0>()) * Vector4(mTextureScale);
    outShadingData.frame[0] = VECTOR_X;
    outShadingData.frame[1] = VECTOR_Z;
    outShadingData.frame[2] = VECTOR_Y;
}


} // namespace rt
