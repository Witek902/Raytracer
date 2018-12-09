#include "PCH.h"
#include "PointLight.h"
#include "../../Rendering/Context.h"
#include "../../Rendering/ShadingData.h"

namespace rt {

using namespace math;

PointLight::PointLight(const math::Vector4& position, const math::Vector4& color)
    : ILight(color)
    , mPosition(position)
{
    RT_ASSERT(mPosition.IsValid());
}

const Box PointLight::GetBoundingBox() const
{
    return Box(mPosition, mPosition);
}

bool PointLight::TestRayHit(const math::Ray& ray, Float& outDistance) const
{
    RT_UNUSED(ray);
    RT_UNUSED(outDistance);

    // we assume that a ray can never hit a point light source
    return false;
}

const Color PointLight::Illuminate(IlluminateParam& param) const
{
    param.outDirectionToLight = mPosition - param.shadingData.position;
    const float sqrDistance = param.outDirectionToLight.SqrLength3();

    param.outDirectPdfW = sqrDistance;
    param.outDistance = std::sqrt(sqrDistance);
    param.outDirectionToLight /= param.outDistance;

    return Color::SampleRGB(param.context.wavelength, mColor);
}

const Color PointLight::GetRadiance(RenderingContext& context, const math::Vector4& rayDirection, const math::Vector4& hitPoint, Float* outDirectPdfA) const
{
    RT_UNUSED(rayDirection);
    RT_UNUSED(hitPoint);
    RT_UNUSED(context);
    RT_UNUSED(outDirectPdfA);

    RT_FATAL("Cannot hit point light");
    return Color();
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
