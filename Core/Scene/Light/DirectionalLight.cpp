#include "PCH.h"
#include "DirectionalLight.h"
#include "../Camera.h"
#include "../../Rendering/Context.h"
#include "../../Rendering/ShadingData.h"
#include "../../Math/Geometry.h"
#include "../../Math/SamplingHelpers.h"
#include "../../Math/Transcendental.h"

namespace rt {

using namespace math;

static constexpr const float SceneRadius = 30.0f; // TODO

DirectionalLight::DirectionalLight(const math::Vector4& direction, const math::Vector4& color, const float angle)
    : ILight(color)
    , mDirection(direction.Normalized3())
{
    RT_ASSERT(mDirection.IsValid());
    RT_ASSERT(Abs(mDirection.SqrLength3() - 1.0f) < 0.001f);
    RT_ASSERT(angle >= 0.0f && angle < RT_2PI);

    mCosAngle = cosf(angle);
    mIsDelta = mCosAngle > CosEpsilon;
}

ILight::Type DirectionalLight::GetType() const
{
    return Type::Directional;
}

const Box DirectionalLight::GetBoundingBox() const
{
    return Box::Full();
}

bool DirectionalLight::TestRayHit(const math::Ray& ray, float& outDistance) const
{
    if (!mIsDelta)
    {
        if (Vector4::Dot3(ray.dir, mDirection) < -mCosAngle)
        {
            outDistance = BackgroundLightDistance;
            return true;
        }
    }

    return false;
}

const Vector4 DirectionalLight::SampleDirection(const Float2 sample, float& outPdf) const
{
    Vector4 sampledDirection;

    if (mIsDelta)
    {
        outPdf = 1.0f;
        sampledDirection = -mDirection;
    }
    else
    {
        outPdf = SphereCapPdf(mCosAngle);

        const float phi = RT_2PI * sample.y;
        const Vector4 sinCosPhi = SinCos(phi);

        float cosTheta = Lerp(mCosAngle, 1.0f, sample.x);
        float sinThetaSqr = 1.0f - Sqr(cosTheta);
        float sinTheta = sqrtf(sinThetaSqr);

        // generate ray direction in the cone uniformly
        const Vector4 w = -mDirection;
        Vector4 u, v;
        BuildOrthonormalBasis(w, u, v);
        sampledDirection = (u * sinCosPhi.y + v * sinCosPhi.x) * sinTheta + w * cosTheta;
        sampledDirection.Normalize3();

        RT_ASSERT(sampledDirection.IsValid());
    }

    return sampledDirection;
}

const RayColor DirectionalLight::Illuminate(const IlluminateParam& param, IlluminateResult& outResult) const
{
    outResult.directionToLight = SampleDirection(param.sample, outResult.directPdfW);
    outResult.emissionPdfW = outResult.directPdfW * UniformCirclePdf(SceneRadius);
    outResult.cosAtLight = 1.0f;
    outResult.distance = BackgroundLightDistance;

    return RayColor::Resolve(param.wavelength, GetColor());
}

const RayColor DirectionalLight::GetRadiance(RenderingContext& context, const math::Ray& ray, const math::Vector4& hitPoint, float* outDirectPdfA, float* outEmissionPdfW) const
{
    RT_UNUSED(hitPoint);

    if (mIsDelta)
    {
        // can't hit delta light
        return RayColor::Zero();
    }

    if (Vector4::Dot3(ray.dir, mDirection) > -mCosAngle)
    {
        return RayColor::Zero();
    }

    const float directPdf = SphereCapPdf(mCosAngle);

    if (outDirectPdfA)
    {
        *outDirectPdfA = directPdf;
    }

    if (outEmissionPdfW)
    {
        *outEmissionPdfW = directPdf * UniformCirclePdf(SceneRadius);
    }

    return RayColor::Resolve(context.wavelength, GetColor());
}

const RayColor DirectionalLight::Emit(const EmitParam& param, EmitResult& outResult) const
{
    outResult.direction = -SampleDirection(param.sample, outResult.directPdfA);

    // generate random origin
    const Vector4 uv = SamplingHelpers::GetCircle(param.sample2);
    {
        Vector4 u, v;
        BuildOrthonormalBasis(mDirection, u, v);
        outResult.position = (u * uv.x + v * uv.y - mDirection) * SceneRadius;
    }

    outResult.cosAtLight = 1.0f;
    outResult.emissionPdfW = outResult.directPdfA * UniformCirclePdf(SceneRadius);

    return RayColor::Resolve(param.wavelength, GetColor());
}

bool DirectionalLight::IsFinite() const
{
    return false;
}

bool DirectionalLight::IsDelta() const
{
    return mIsDelta;
}

} // namespace rt
