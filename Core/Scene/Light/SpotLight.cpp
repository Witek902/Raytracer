#include "PCH.h"
#include "SpotLight.h"
#include "../../Rendering/Context.h"
#include "../../Rendering/ShadingData.h"
#include "../../Math/Geometry.h"
#include "../../Math/Transcendental.h"

namespace rt {

using namespace math;

SpotLight::SpotLight(const Vector4& position, const Vector4& direction, const Vector4& color, const float angle)
    : ILight(color)
    , mPosition(position)
    , mDirection(direction.Normalized3())
    , mAngle(angle)
{
    RT_ASSERT(mPosition.IsValid());
    RT_ASSERT(mDirection.IsValid());
    RT_ASSERT(Abs(mDirection.SqrLength3() - 1.0f) < 0.001f);
    RT_ASSERT(angle >= 0.0f && angle < RT_2PI);

    mCosAngle = cosf(angle);
    mIsDelta = mCosAngle > CosEpsilon;
}

ILight::Type SpotLight::GetType() const
{
    return Type::Spot;
}

const Box SpotLight::GetBoundingBox() const
{
    return { mPosition, mPosition };
}

bool SpotLight::TestRayHit(const Ray& ray, float& outDistance) const
{
    RT_UNUSED(ray);
    RT_UNUSED(outDistance);

    // we assume that a ray can never hit a point light source
    return false;
}

const RayColor SpotLight::Illuminate(const IlluminateParam& param, IlluminateResult& outResult) const
{
    outResult.directionToLight = mPosition - param.shadingData.frame.GetTranslation();
    const float sqrDistance = outResult.directionToLight.SqrLength3();

    outResult.directPdfW = sqrDistance;
    outResult.distance = std::sqrt(sqrDistance);
    outResult.directionToLight /= outResult.distance;
    outResult.cosAtLight = 1.0f;
    outResult.emissionPdfW = mIsDelta ? 1.0f : SphereCapPdf(mCosAngle);

    const float angle = Vector4::Dot3(outResult.directionToLight, -mDirection);
    
    if (angle < mCosAngle)
    {
        return RayColor::Zero();
    }

    // TODO IES profile
    return RayColor::Resolve(param.wavelength, GetColor());
}

const RayColor SpotLight::Emit(const EmitParam& param, EmitResult& outResult) const
{
    if (mIsDelta)
    {
        outResult.emissionPdfW = 1.0f;
        outResult.direction = mDirection;
    }
    else
    {
        const float phi = RT_2PI * param.sample.y;
        const Vector4 sinCosPhi = SinCos(phi);

        float cosTheta = Lerp(mCosAngle, 1.0f, param.sample.x);
        float sinThetaSqr = 1.0f - Sqr(cosTheta);
        float sinTheta = sqrtf(sinThetaSqr);

        // generate ray direction in the cone uniformly
        Vector4 u, v;
        BuildOrthonormalBasis(mDirection, u, v);
        outResult.direction = (u * sinCosPhi.y + v * sinCosPhi.x) * sinTheta + mDirection * cosTheta;
        outResult.direction.Normalize3();
        outResult.emissionPdfW = SphereCapPdf(mCosAngle);
    }

    outResult.position = mPosition;
    outResult.directPdfA = 1.0f;
    outResult.cosAtLight = 1.0f;

    // TODO IES profile
    return RayColor::Resolve(param.wavelength, GetColor());
}

bool SpotLight::IsFinite() const
{
    return true;
}

bool SpotLight::IsDelta() const
{
    return mIsDelta;
}

} // namespace rt
