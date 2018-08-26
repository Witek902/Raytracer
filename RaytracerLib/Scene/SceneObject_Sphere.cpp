#include "PCH.h"
#include "SceneObject_Sphere.h"
#include "Math/Geometry.h"
#include "Rendering/ShadingData.h"
#include "Traversal/TraversalContext.h"

namespace rt {

using namespace math;


SphereSceneObject::SphereSceneObject(const float radius, const Material* material)
    : mRadius(radius)
    , mInvRadius(1.0f / radius)
    , mMaterial(material)
{ }

Box SphereSceneObject::GetBoundingBox() const
{
    const Vector4 radius = Vector4(mRadius, mRadius, mRadius, 0.0f);

    const Box localBox(mTransform.GetTranslation() - radius, mTransform.GetTranslation() + radius);

    // TODO include rotation
    return Box(localBox, localBox + mTransformDelta.GetTranslation());
}

void SphereSceneObject::Traverse_Single(const SingleTraversalContext& context, const Uint32 objectID) const
{
    // TODO this breaks transparency, backface culling test is required
    if (context.hitPoint.objectId != objectID)
    {
        float dist;
        if (Intersect_RaySphere(context.ray, mRadius, dist))
        {
            if (dist > 0.0f && dist < context.hitPoint.distance)
            {
                context.hitPoint.distance = dist;
                context.hitPoint.objectId = objectID;
                context.hitPoint.triangleId = 0;
            }
        }
    }
}

void SphereSceneObject::Traverse_Simd8(const SimdTraversalContext& context, const Uint32 objectID) const
{
    RT_UNUSED(objectID);
    RT_UNUSED(context);
    // TODO
}

void SphereSceneObject::Traverse_Packet(const PacketTraversalContext& context, const Uint32 objectID) const
{
    RT_UNUSED(objectID);
    RT_UNUSED(context);
    // TODO
}

void SphereSceneObject::EvaluateShadingData_Single(const HitPoint& hitPoint, ShadingData& outShadingData) const
{
    RT_UNUSED(hitPoint);

    outShadingData.material = mMaterial;

    outShadingData.texCoord = Vector4(); // TODO
    outShadingData.normal = outShadingData.position * mInvRadius;
    outShadingData.tangent = Vector4::Cross3(outShadingData.normal, VECTOR_Y);
    outShadingData.bitangent = Vector4::Cross3(outShadingData.tangent, outShadingData.normal);

    outShadingData.normal.FastNormalize3();
    outShadingData.tangent.FastNormalize3();
    outShadingData.bitangent.FastNormalize3();
}


} // namespace rt
