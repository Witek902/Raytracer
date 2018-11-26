#include "PCH.h"
#include "SceneObject_Plane.h"
#include "Math/Geometry.h"
#include "Rendering/ShadingData.h"
#include "Traversal/TraversalContext.h"

namespace rt {

using namespace math;

PlaneSceneObject::PlaneSceneObject(const math::Vector4 texScale)
    : mTextureScale(texScale)
{ }

Box PlaneSceneObject::GetBoundingBox() const
{
    return Box::Full();
}

void PlaneSceneObject::Traverse_Single(const SingleTraversalContext& context, const Uint32 objectID) const
{
    if (Abs(context.ray.dir.y) > RT_EPSILON)
    {
        const float t = -context.ray.origin.y * context.ray.invDir.y;

        if (t > 0.0f && t < context.hitPoint.distance)
        {
            context.hitPoint.distance = t;
            context.hitPoint.objectId = objectID;
            context.hitPoint.triangleId = 0;
        }
    }
}

bool PlaneSceneObject::Traverse_Shadow_Single(const SingleTraversalContext& context) const
{
    if (Abs(context.ray.dir.y) > RT_EPSILON)
    {
        const float t = -context.ray.origin.y * context.ray.invDir.y;

        if (t > 0.0f && t < context.hitPoint.distance)
        {
            context.hitPoint.distance = t;
            return true;
        }
    }

    return false;
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
    outShadingData.texCoord = Vector4(outShadingData.position.x, outShadingData.position.z, 0.0f, 0.0f) * mTextureScale;
    outShadingData.normal = VECTOR_Y;
    outShadingData.tangent = VECTOR_X;
    outShadingData.bitangent = Vector4::Cross3(outShadingData.tangent, outShadingData.normal);
}


} // namespace rt
