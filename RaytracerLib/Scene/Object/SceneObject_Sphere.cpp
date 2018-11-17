#include "PCH.h"
#include "SceneObject_Sphere.h"
#include "Math/Geometry.h"
#include "Rendering/ShadingData.h"
#include "Traversal/TraversalContext.h"

namespace rt {

using namespace math;


SphereSceneObject::SphereSceneObject(const float radius, const Material* material)
    : mMaterial(material)
    , mRadius(radius)
    , mInvRadius(1.0f / radius)
{ }

Box SphereSceneObject::GetBoundingBox() const
{
    const Vector4 radius = Vector4(mRadius, mRadius, mRadius, 0.0f);

    const Box localBox(mTransform.GetTranslation() - radius, mTransform.GetTranslation() + radius);
    return Box(localBox, localBox + mLinearVelocity);
}

void SphereSceneObject::Traverse_Single(const SingleTraversalContext& context, const Uint32 objectID) const
{
    const Vector4 d = -context.ray.origin;
    const double v = Vector4::Dot3(context.ray.dir, d);
    const double det = (double)(mRadius * mRadius) - (double)Vector4::Dot3(d, d) + v * v;

    if (det > 0.0)
    {
        const double sqrtDet = sqrt(det);

        const float nearDist = (float)(v - sqrtDet);
        if (nearDist > 0.0f && nearDist < context.hitPoint.distance && (objectID != context.hitPoint.objectId || context.hitPoint.triangleId != 0))
        {
            context.hitPoint.distance = nearDist;
            context.hitPoint.objectId = objectID;
            context.hitPoint.triangleId = 0;
            return;
        }

        const float farDist = (float)(v + sqrtDet);
        if (farDist > 0.0f && farDist < context.hitPoint.distance && (objectID != context.hitPoint.objectId || context.hitPoint.triangleId != 0))
        {
            context.hitPoint.distance = farDist;
            context.hitPoint.objectId = objectID;
            context.hitPoint.triangleId = 1;
            return;
        }
    }
}

bool SphereSceneObject::Traverse_Shadow_Single(const SingleTraversalContext& context) const
{
    const Vector4 d = -context.ray.origin;
    const double v = Vector4::Dot3(context.ray.dir, d);
    const double det = (double)(mRadius * mRadius) - (double)Vector4::Dot3(d, d) + v * v;

    if (det > 0.0)
    {
        const double sqrtDet = sqrt(det);

        const float nearDist = (float)(v - sqrtDet);
        if (nearDist > 0.0f && nearDist < context.hitPoint.distance)
        {
            context.hitPoint.distance = nearDist;
            return true;
        }

        const float farDist = (float)(v + sqrtDet);
        if (farDist > 0.0f && farDist < context.hitPoint.distance)
        {
            context.hitPoint.distance = farDist;
            return true;
        }
    }

    return false;
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

    outShadingData.texCoord = Vector4::Zero(); // TODO
    outShadingData.normal = outShadingData.position * mInvRadius;
    outShadingData.tangent = Vector4::Cross3(outShadingData.normal, VECTOR_Y);
    outShadingData.bitangent = Vector4::Cross3(outShadingData.tangent, outShadingData.normal);

    outShadingData.normal.FastNormalize3();
    outShadingData.tangent.FastNormalize3();
    outShadingData.bitangent.FastNormalize3();
}


} // namespace rt
