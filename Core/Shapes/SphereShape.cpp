#include "PCH.h"
#include "SphereShape.h"
#include "Rendering/ShadingData.h"
#include "Traversal/TraversalContext.h"
#include "Math/Geometry.h"
#include "Math/SamplingHelpers.h"
#include "Math/Transcendental.h"

namespace rt {

using namespace math;

SphereShape::SphereShape(const float radius)
    : mRadius(radius)
    , mRadiusD(radius)
    , mInvRadius(1.0f / radius)
{ }

const Box SphereShape::GetBoundingBox() const
{
    return Box(Vector4::Zero(), mRadius);
}

float SphereShape::GetSurfaceArea() const
{
    return 4.0f * RT_PI * Sqr(mRadius);
}

bool SphereShape::Intersect(const math::Ray& ray, ShapeIntersection& outResult) const
{
    const double v = Vector4::Dot3(ray.dir, -ray.origin);
    const double det = mRadiusD * mRadiusD - (double)ray.origin.SqrLength3() + v * v;

    if (det <= 0.0)
    {
        return false;
    }

    const double sqrtDet = sqrt(det);
    outResult.nearDist = (float)(v - sqrtDet);
    outResult.farDist = (float)(v + sqrtDet);

    outResult.subObjectId = 0;

    return outResult.farDist > outResult.nearDist;
}

const Vector4 SphereShape::Sample(const Float3& u, math::Vector4* outNormal, float* outPdf) const
{
    if (outPdf)
    {
        *outPdf = 1.0f / GetSurfaceArea();
    }

    const Vector4 point = SamplingHelpers::GetSphere(u);

    if (outNormal)
    {
        *outNormal = point;
    }

    return point * mRadius;
}

bool SphereShape::Sample(const Vector4& ref, const Float3& u, ShapeSampleResult& result) const
{
    const Vector4 centerDir = -ref; // direction to light center
    const float centerDistSqr = centerDir.SqrLength3();
    const float centerDist = sqrtf(centerDistSqr);

    if (centerDistSqr < Sqr(mRadius))
    {
        // TODO illuminate inside?
        return false;
    }

    const float phi = RT_2PI * u.y;
    const Vector4 sinCosPhi = SinCos(phi);

    float sinThetaMaxSqr = Sqr(mRadius) / centerDistSqr;
    float cosThetaMax = sqrtf(1.0f - Clamp(sinThetaMaxSqr, 0.0f, 1.0f));
    float cosTheta = Lerp(cosThetaMax, 1.0f, u.x);
    float sinThetaSqr = 1.0f - Sqr(cosTheta);
    float sinTheta = sqrtf(sinThetaSqr);

    // generate ray direction in the cone uniformly
    const Vector4 w = centerDir / centerDist;
    Vector4 tangent, bitangent;
    BuildOrthonormalBasis(w, tangent, bitangent);
    result.direction = (tangent * sinCosPhi.y + bitangent * sinCosPhi.x) * sinTheta + w * cosTheta;
    result.direction.Normalize3();

    // calculate distance to hit point
    result.distance = centerDist * cosTheta - sqrtf(Max(0.0f, Sqr(mRadius) - centerDistSqr * sinThetaSqr));

    result.cosAtSurface = cosTheta;

    if (cosThetaMax > 0.999999f)
    {
        result.pdf = FLT_MAX;
    }
    else
    {
        result.pdf = SphereCapPdf(cosThetaMax);
    }

    return true;
}

float SphereShape::Pdf(const math::Vector4& ref, const math::Vector4& point) const
{
    const Vector4 rayDir = (point - ref).Normalized3();

    const Vector4 centerDir = -ref; // direction to light center
    const float centerDistSqr = centerDir.SqrLength3();
    const Vector4 normal = point.Normalized3();
    const float cosAtLight = Max(0.0f, Vector4::Dot3(-rayDir, normal));

    const float sinThetaMaxSqr = Clamp(Sqr(mRadius) / centerDistSqr, 0.0f, 1.0f);
    const float cosThetaMax = sqrtf(1.0f - sinThetaMaxSqr);
    const float pdfW = SphereCapPdf(cosThetaMax);

    // TODO may convert W->A unnecessary
    return pdfW * cosAtLight / (point - ref).SqrLength3();
}

/*
void SphereShape::Traverse_Packet(const PacketTraversalContext& context, const Uint32 objectID, const Uint32 numActiveGroups) const
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

        const Vector8 uCoord = Vector8::Zero(); // TODO
        const Vector8 vCoord = Vector8::Zero(); // TODO

        context.StoreIntersection(rayGroup, t, uCoord, vCoord, distMask, objectID);
    }
}
*/

void SphereShape::EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outData) const
{
    RT_UNUSED(hitPoint);

    outData.texCoord = CartesianToSphericalCoordinates(-outData.frame.GetTranslation());
    outData.frame[2] = outData.frame.GetTranslation() * mInvRadius;

    // equivalent of: Vector4::Cross3(outData.normal, VECTOR_Y);
    outData.frame[0] = (outData.frame[2].Swizzle<2,0,0,0>() & Vector4::MakeMask<1,0,1,0>()).ChangeSign<1,0,0,0>();

    outData.frame[1] = -Vector4::Cross3(outData.frame[0], outData.frame[2]);

    // TODO is that needed?
    outData.frame[0].FastNormalize3();
    outData.frame[1].FastNormalize3();
    outData.frame[2].FastNormalize3();
}


} // namespace rt
