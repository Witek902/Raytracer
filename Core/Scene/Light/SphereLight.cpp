#include "PCH.h"
#include "SphereLight.h"
#include "../../Rendering/Context.h"
#include "../../Rendering/ShadingData.h"
#include "../../Math/Geometry.h"

namespace rt {

using namespace math;

SphereLight::SphereLight(const math::Vector4& pos, float radius, const math::Vector4& color)
    : ILight(color)
    , mPosition(pos)
    , mRadius(Abs(radius))
    , mRadiusSqr(radius * radius)
{
    RT_ASSERT(pos.IsValid());
    RT_ASSERT(IsValid(radius));
}

const Box SphereLight::GetBoundingBox() const
{
    return Box(mPosition, mRadius);
}

bool SphereLight::TestRayHit(const math::Ray& ray, float& outDistance) const
{
    // TODO optimize

    const Vector4 d = mPosition - ray.origin;
    const double v = Vector4::Dot3(ray.dir, d);
    const double det = (double)mRadiusSqr - (double)d.SqrLength3() + v * v;

    if (det > 0.0)
    {
        const double sqrtDet = sqrt(det);

        const float nearDist = (float)(v - sqrtDet);
        if (nearDist > 0.0f)
        {
            outDistance = nearDist;
            return true;
        }

        const float farDist = (float)(v + sqrtDet);
        if (farDist > 0.0f)
        {
            outDistance = farDist;
            return true;
        }
    }

    return false;
}

const RayColor SphereLight::Illuminate(IlluminateParam& param) const
{
    const Vector4 centerDir = mPosition - param.shadingData.frame.GetTranslation(); // direction to light center
    const float centerDistSqr = centerDir.SqrLength3();
    const float centerDist = sqrtf(centerDistSqr);

    if (centerDistSqr < mRadiusSqr)
    {
        // TODO illuminate inside?
        return RayColor::Zero();
    }

    const Float2 uv = param.context.randomGenerator.GetFloat2();
    const float phi = RT_2PI * uv.y;

    float sinThetaMaxSqr = mRadiusSqr / centerDistSqr;
    float cosThetaMax = sqrtf(1.0f - Clamp(sinThetaMaxSqr, 0.0f, 1.0f));
    float cosTheta = Lerp(cosThetaMax, 1.0f, uv.x);
    float sinThetaSqr = 1.0f - Sqr(cosTheta);
    float sinTheta = sqrtf(sinThetaSqr);

    // generate ray direction in the cone uniformly
    const Vector4 w = centerDir / centerDist;
    Vector4 u, v;
    BuildOrthonormalBasis(w, u, v);
    param.outDirectionToLight = (u * cosf(phi) + v * sinf(phi)) * sinTheta + w * cosTheta;
    param.outDirectionToLight.Normalize3();

    // calculate distance to hit point
    param.outDistance = centerDist * cosTheta - sqrtf(Max(0.0f, mRadiusSqr - centerDistSqr * sinThetaSqr));

    if (cosThetaMax > 0.999999f)
    {
        param.outDirectPdfW = FLT_MAX;
    }
    else
    {
        param.outDirectPdfW = SphereCapPdf(cosThetaMax);
    }

    param.outEmissionPdfW = 0.0f; // TODO BDPT

    return RayColor::Resolve(param.context.wavelength, mColor);
}

const RayColor SphereLight::GetRadiance(RenderingContext& context, const math::Vector4& rayDirection, const math::Vector4& hitPoint, float* outDirectPdfA, float* outEmissionPdfW) const
{
    RT_UNUSED(rayDirection);

    const Vector4 centerDir = mPosition - hitPoint; // direction to light center
    const float centerDistSqr = centerDir.SqrLength3();

    if (outDirectPdfA)
    {
        const float sinThetaSqr = Clamp(mRadiusSqr / centerDistSqr, 0.0f, 1.0f);
        const float cosTheta = sqrtf(1.0f - sinThetaSqr);
        *outDirectPdfA = SphereCapPdf(cosTheta);
    }

    if (outEmissionPdfW)
    {
        // TODO
        RT_FATAL("Not implemented");
    }

    return RayColor::Resolve(context.wavelength, mColor);
}

const RayColor SphereLight::Emit(RenderingContext& ctx, EmitResult& outResult) const
{
    // TODO
    RT_UNUSED(ctx);
    RT_UNUSED(outResult);
    return RayColor::Zero();
}

const Vector4 SphereLight::GetNormal(const Vector4& hitPoint) const
{
    return (hitPoint - mPosition).Normalized3();
}

bool SphereLight::IsFinite() const
{
    return true;
}

bool SphereLight::IsDelta() const
{
    return false;
}

} // namespace rt
