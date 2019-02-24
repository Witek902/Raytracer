#include "PCH.h"
#include "DirectionalLight.h"
#include "../Camera.h"
#include "../../Rendering/Context.h"
#include "../../Rendering/ShadingData.h"
#include "../../Math/Geometry.h"

namespace rt {

using namespace math;

static constexpr const float CosEpsilon = 0.999f;
static constexpr const float SceneRadius = 5.8f;

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

const Vector4 DirectionalLight::SampleDirection(RenderingContext& context, float& outPdf) const
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

        const Float2 uv = context.randomGenerator.GetFloat2();
        const float phi = RT_2PI * uv.y;

        float cosTheta = Lerp(mCosAngle, 1.0f, uv.x);
        float sinThetaSqr = 1.0f - Sqr(cosTheta);
        float sinTheta = sqrtf(sinThetaSqr);

        // generate ray direction in the cone uniformly
        const Vector4 w = -mDirection;
        Vector4 u, v;
        BuildOrthonormalBasis(w, u, v);
        sampledDirection = (u * cosf(phi) + v * sinf(phi)) * sinTheta + w * cosTheta;
        sampledDirection.Normalize3();
    }

    return sampledDirection;
}

const RayColor DirectionalLight::Illuminate(IlluminateParam& param) const
{
    param.outDirectionToLight = SampleDirection(param.context, param.outDirectPdfW);
    param.outEmissionPdfW = param.outDirectPdfW * UniformCirclePdf(SceneRadius);
    param.outCosAtLight = 1.0f;
    param.outDistance = BackgroundLightDistance;

    return RayColor::Resolve(param.context.wavelength, mColor);
}

const RayColor DirectionalLight::GetRadiance(RenderingContext& context, const math::Vector4& rayDirection, const math::Vector4& hitPoint, float* outDirectPdfA, float* outEmissionPdfW) const
{
    RT_UNUSED(hitPoint);

    if (mIsDelta)
    {
        // can't hit delta light
        return RayColor::Zero();
    }

    if (Vector4::Dot3(rayDirection, mDirection) > -mCosAngle)
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

    return RayColor::Resolve(context.wavelength, mColor);
}

const RayColor DirectionalLight::Emit(RenderingContext& ctx, EmitResult& outResult) const
{
    outResult.direction = -SampleDirection(ctx, outResult.directPdfA);

    // generate random origin
    const Vector4 uv = ctx.randomGenerator.GetCircle();
    {
        Vector4 u, v;
        BuildOrthonormalBasis(mDirection, u, v);
        outResult.position = (u * uv.x + v * uv.y - mDirection) * SceneRadius;
    }

    outResult.cosAtLight = 1.0f;
    outResult.emissionPdfW = outResult.directPdfA * UniformCirclePdf(SceneRadius);

    return RayColor::Resolve(ctx.wavelength, mColor);
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
