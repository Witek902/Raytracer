#include "PCH.h"
#include "PointLight.h"
#include "../../Rendering/Context.h"
#include "../../Rendering/ShadingData.h"
#include "../../Math/SamplingHelpers.h"

namespace rt {

using namespace math;

PointLight::PointLight(const math::Vector4& position, const math::Vector4& color)
    : ILight(color)
    , mPosition(position)
{
    RT_ASSERT(mPosition.IsValid());
}

ILight::Type PointLight::GetType() const
{
    return Type::Point;
}

const Box PointLight::GetBoundingBox() const
{
    return Box(mPosition, mPosition);
}

bool PointLight::TestRayHit(const math::Ray& ray, float& outDistance) const
{
    RT_UNUSED(ray);
    RT_UNUSED(outDistance);

    // we assume that a ray can never hit a point light source
    return false;
}

const RayColor PointLight::Illuminate(const IlluminateParam& param, IlluminateResult& outResult) const
{
    outResult.directionToLight = mPosition - param.shadingData.frame.GetTranslation();
    const float sqrDistance = outResult.directionToLight.SqrLength3();

    outResult.directPdfW = sqrDistance;
    outResult.emissionPdfW = RT_INV_PI / 4.0f;
    outResult.distance = std::sqrt(sqrDistance);
    outResult.directionToLight /= outResult.distance;
    outResult.cosAtLight = 1.0f;

    return RayColor::Resolve(param.wavelength, GetColor());
}

const RayColor PointLight::Emit(const EmitParam& param, EmitResult& outResult) const
{
    outResult.position = mPosition;
    outResult.direction = SamplingHelpers::GetSphere(param.sample);
    outResult.emissionPdfW = RT_INV_PI / 4.0f;
    outResult.directPdfA = 1.0f;
    outResult.cosAtLight = 1.0f;

    // TODO texture
    return RayColor::Resolve(param.wavelength, GetColor());
}

bool PointLight::IsFinite() const
{
    return true;
}

bool PointLight::IsDelta() const
{
    return true;
}

} // namespace rt
