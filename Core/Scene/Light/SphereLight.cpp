#include "PCH.h"
#include "SphereLight.h"
#include "../../Rendering/Context.h"
#include "../../Rendering/ShadingData.h"
#include "../../Math/Geometry.h"
#include "../../Math/SamplingHelpers.h"
#include "../../Math/Transcendental.h"

namespace rt {

using namespace math;

SphereLight::SphereLight(const math::Vector4& pos, float radius, const math::Vector4& color)
    : ILight(color)
    , mPosition(pos)
    , mRadius(Abs(radius))
    , mRadiusSqr(Sqr(radius))
{
    RT_ASSERT(pos.IsValid());
    RT_ASSERT(IsValid(radius));
}

ILight::Type SphereLight::GetType() const
{
    return Type::Sphere;
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

const RayColor SphereLight::Illuminate(const IlluminateParam& param, IlluminateResult& outResult) const
{
    const Vector4 centerDir = mPosition - param.shadingData.frame.GetTranslation(); // direction to light center
    const float centerDistSqr = centerDir.SqrLength3();
    const float centerDist = sqrtf(centerDistSqr);

    if (centerDistSqr < mRadiusSqr)
    {
        // TODO illuminate inside?
        return RayColor::Zero();
    }

    const float phi = RT_2PI * param.sample.y;
    const Vector4 sinCosPhi = SinCos(phi);

    float sinThetaMaxSqr = mRadiusSqr / centerDistSqr;
    float cosThetaMax = sqrtf(1.0f - Clamp(sinThetaMaxSqr, 0.0f, 1.0f));
    float cosTheta = Lerp(cosThetaMax, 1.0f, param.sample.x);
    float sinThetaSqr = 1.0f - Sqr(cosTheta);
    float sinTheta = sqrtf(sinThetaSqr);

    // generate ray direction in the cone uniformly
    const Vector4 w = centerDir / centerDist;
    Vector4 u, v;
    BuildOrthonormalBasis(w, u, v);
    outResult.directionToLight = (u * sinCosPhi.y + v * sinCosPhi.x) * sinTheta + w * cosTheta;
    outResult.directionToLight.Normalize3();

    // calculate distance to hit point
    outResult.distance = centerDist * cosTheta - sqrtf(Max(0.0f, mRadiusSqr - centerDistSqr * sinThetaSqr));

    if (cosThetaMax > 0.999999f)
    {
        outResult.directPdfW = FLT_MAX;
    }
    else
    {
        outResult.directPdfW = SphereCapPdf(cosThetaMax);
    }

    outResult.cosAtLight = cosTheta;
    //outResult.emissionPdfW = UniformSpherePdf(mRadius) * (cosTheta * RT_INV_PI);

    return RayColor::Resolve(param.wavelength, GetColor());
}

const RayColor SphereLight::GetRadiance(RenderingContext& context, const math::Ray& ray, const math::Vector4& hitPoint, float* outDirectPdfA, float* outEmissionPdfW) const
{
    RT_UNUSED(hitPoint);

    const Vector4 centerDir = mPosition - ray.origin; // direction to light center
    const float centerDistSqr = centerDir.SqrLength3();
    const float cosAtLight = Max(0.0f, Vector4::Dot3(-ray.dir, GetNormal(hitPoint)));

    if (outDirectPdfA)
    {
        const float sinThetaMaxSqr = Clamp(mRadiusSqr / centerDistSqr, 0.0f, 1.0f);
        const float cosThetaMax = sqrtf(1.0f - sinThetaMaxSqr);
        const float pdfW = SphereCapPdf(cosThetaMax);

        // TODO may convert W->A unnecessary
        *outDirectPdfA = pdfW * cosAtLight / (hitPoint - ray.origin).SqrLength3();
    }

    if (outEmissionPdfW)
    {
        RT_FATAL("Not implemented");
        //*outEmissionPdfW = UniformSpherePdf(mRadius) * (cosAtLight * RT_INV_PI);
    }

    return RayColor::Resolve(context.wavelength, GetColor());
}

const RayColor SphereLight::Emit(const EmitParam& param, EmitResult& outResult) const
{
    RT_FATAL("Not implemented");

    RT_UNUSED(param);
    RT_UNUSED(outResult);

    //const Vector4 normal = SamplingHelpers::GetSphere(param.sample);
    //outResult.position = Vector4::MulAndAdd(normal, mRadius, mPosition);

    //const Vector4 dirLocalSpace = SamplingHelpers::GetHemishpereCos(param.sample2);
    //{
    //    Vector4 u, v;
    //    BuildOrthonormalBasis(normal, u, v);
    //    outResult.direction = u * dirLocalSpace.x + v * dirLocalSpace.y + normal * dirLocalSpace.z;
    //}

    //outResult.directPdfA = UniformSpherePdf(mRadius);
    //outResult.emissionPdfW = UniformSpherePdf(mRadius) * (dirLocalSpace.z * RT_INV_PI);
    //outResult.cosAtLight = dirLocalSpace.z;

    //return RayColor::Resolve(param.wavelength, mColor) * dirLocalSpace.z;

    return RayColor::Zero();
}

const Vector4 SphereLight::GetNormal(const Vector4& hitPoint) const
{
    return (hitPoint - mPosition).Normalized3();
}

ILight::Flags SphereLight::GetFlags() const
{
    return Flag_IsFinite;
}

} // namespace rt
