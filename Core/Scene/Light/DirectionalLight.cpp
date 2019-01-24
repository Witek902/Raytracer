#include "PCH.h"
#include "DirectionalLight.h"
#include "../../Rendering/Context.h"
#include "../../Math/Geometry.h"

namespace rt {

using namespace math;

static constexpr const Float CosEpsilon = 0.999f;

DirectionalLight::DirectionalLight(const math::Vector4& direction, const math::Vector4& color, const Float angle)
    : ILight(color)
    , mDirection(direction.Normalized3())
{
    RT_ASSERT(mDirection.IsValid());
    RT_ASSERT(Abs(mDirection.SqrLength3() - 1.0f) < 0.001f);
    RT_ASSERT(angle >= 0.0f && angle < RT_2PI);

    mCosAngle = cosf(angle);
}

const Box DirectionalLight::GetBoundingBox() const
{
    return Box::Full();
}

bool DirectionalLight::TestRayHit(const math::Ray& ray, Float& outDistance) const
{
    if (mCosAngle < CosEpsilon)
    {
        if (Vector4::Dot3(ray.dir, mDirection) < -mCosAngle)
        {
            outDistance = BackgroundLightDistance;
            return true;
        }
    }

    return false;
}

const Color DirectionalLight::Illuminate(IlluminateParam& param) const
{
    if (mCosAngle < CosEpsilon)
    {
        const Float2 uv = param.context.randomGenerator.GetFloat2();
        const Float phi = RT_2PI * uv.y;

        float cosTheta = Lerp(mCosAngle, 1.0f, uv.x);
        float sinThetaSqr = 1.0f - Sqr(cosTheta);
        float sinTheta = sqrtf(sinThetaSqr);

        // generate ray direction in the cone uniformly
        const Vector4 w = -mDirection;
        Vector4 u, v;
        BuildOrthonormalBasis(w, u, v);
        param.outDirectionToLight = (u * cosf(phi) + v * sinf(phi)) * sinTheta + w * cosTheta;
        param.outDirectionToLight.Normalize3();
        param.outDirectPdfW = SphereCapPdf(mCosAngle);
    }
    else
    {
        param.outDirectionToLight = -mDirection;
        param.outDirectPdfW = 1.0f;
    }

    param.outDistance = BackgroundLightDistance;

    return Color::SampleRGB(param.context.wavelength, mColor);
}

const Color DirectionalLight::GetRadiance(RenderingContext& context, const math::Vector4& rayDirection, const math::Vector4& hitPoint, Float* outDirectPdfA) const
{
    RT_UNUSED(hitPoint);

    if (mCosAngle < CosEpsilon)
    {
        if (Vector4::Dot3(rayDirection, mDirection) < -mCosAngle)
        {
            if (outDirectPdfA)
            {
                *outDirectPdfA = SphereCapPdf(mCosAngle);
            }

            return Color::SampleRGB(context.wavelength, mColor);
        }
    }

    return Color::Zero();
}

bool DirectionalLight::IsFinite() const
{
    return false;
}

bool DirectionalLight::IsDelta() const
{
    return mCosAngle > CosEpsilon;
}

} // namespace rt
