#include "PCH.h"
#include "SceneObject_Sphere.h"
#include "Math/Geometry.h"
#include "Rendering/ShadingData.h"
#include "Rendering/Context.h"
#include "Traversal/TraversalContext.h"

namespace rt {

using namespace math;


SphereSceneObject::SphereSceneObject(const float radius)
    : mRadius(radius)
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
    const double v = Vector4::Dot3(context.ray.dir, -context.ray.origin);
    const double det = (double)(mRadius * mRadius) - (double)context.ray.origin.SqrLength3() + v * v;

    if (det > 0.0)
    {
        const double sqrtDet = sqrt(det);

        const float nearDist = (float)(v - sqrtDet);
        if (nearDist > 0.0f && nearDist < context.hitPoint.distance)
        {
            context.hitPoint.distance = nearDist;
            context.hitPoint.objectId = objectID;
            context.hitPoint.subObjectId = 0;
            return;
        }

        const float farDist = (float)(v + sqrtDet);
        if (farDist > 0.0f && farDist < context.hitPoint.distance)
        {
            context.hitPoint.distance = farDist;
            context.hitPoint.objectId = objectID;
            context.hitPoint.subObjectId = 0;
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

void SphereSceneObject::Traverse_Packet(const PacketTraversalContext& context, const Uint32 objectID, const Uint32 numActiveGroups) const
{
    for (Uint32 i = 0; i < numActiveGroups; ++i)
    {
        RayGroup& rayGroup = context.ray.groups[context.context.activeGroupsIndices[i]];
        const Ray_Simd8& ray = rayGroup.rays[1];

        const Vector8 v = Vector3x8::Dot(ray.dir, -ray.origin);
        const Vector8 det = Vector8(mRadius * mRadius) - Vector3x8::Dot(ray.origin, ray.origin) + v * v;

        const VectorBool8 detSign = det > Vector8::Zero();
        if (detSign.None())
        {
            continue;
        }

        const Vector8 sqrtDet = Vector8::Sqrt(det);
        const Vector8 nearDist = v - sqrtDet;
        const Vector8 farDist = v + sqrtDet;
        const Vector8 t = Vector8::Select(nearDist, farDist, nearDist < Vector8::Zero());

        const VectorBool8 distMask = detSign & (t > Vector8::Zero()) & (t < rayGroup.maxDistances);

        context.StoreIntersection(rayGroup, t, distMask, objectID);
    }
}

void SphereSceneObject::EvaluateShadingData_Single(const HitPoint& hitPoint, ShadingData& outShadingData) const
{
    RT_UNUSED(hitPoint);

    outShadingData.material = mDefaultMaterial.get();

    outShadingData.texCoord = Vector4::Zero(); // TODO
    outShadingData.normal = outShadingData.position * mInvRadius;

    // equivalent of: Vector4::Cross3(outShadingData.normal, VECTOR_Y);
    outShadingData.tangent = (outShadingData.normal.Swizzle<2,0,0,0>() & Vector4::MakeMask<1,0,1,0>()).ChangeSign<1,0,0,0>();

    outShadingData.bitangent = Vector4::Cross3(outShadingData.tangent, outShadingData.normal);

    outShadingData.normal.FastNormalize3();
    outShadingData.tangent.FastNormalize3();
    outShadingData.bitangent.FastNormalize3();
}


} // namespace rt
