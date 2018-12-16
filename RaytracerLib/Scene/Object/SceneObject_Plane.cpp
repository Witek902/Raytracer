#include "PCH.h"
#include "SceneObject_Plane.h"
#include "Math/Geometry.h"
#include "Rendering/ShadingData.h"
#include "Traversal/TraversalContext.h"

namespace rt {

using namespace math;

PlaneSceneObject::PlaneSceneObject(const Float2 size, const Float2 texScale)
    : mSize(size)
    , mTextureScale(texScale)
{ }

Box PlaneSceneObject::GetBoundingBox() const
{
    // TODO
    return Box::Full();
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

void PlaneSceneObject::Traverse_Simd8(const SimdTraversalContext& context, const Uint32 objectID) const
{
    RT_UNUSED(objectID);
    (void)context;
    // TODO
}

void PlaneSceneObject::Traverse_Packet(const PacketTraversalContext& context, const Uint32 objectID) const
{
    RT_UNUSED(objectID);
    (void)context;
    // TODO
}

void PlaneSceneObject::EvaluateShadingData_Single(const HitPoint& hitPoint, ShadingData& outShadingData) const
{
    RT_UNUSED(hitPoint);

    outShadingData.material = mDefaultMaterial.get();
    outShadingData.texCoord = Vector4(outShadingData.position.x, outShadingData.position.z, 0.0f, 0.0f) * Vector4(mTextureScale);
    outShadingData.tangent = VECTOR_X;
    outShadingData.normal = VECTOR_Y;
    outShadingData.bitangent = VECTOR_Z;
}


} // namespace rt
