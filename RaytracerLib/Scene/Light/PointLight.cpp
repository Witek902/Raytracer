#include "PCH.h"
#include "PointLight.h"
#include "../../Rendering/Context.h"

namespace rt {

using namespace math;

const Box PointLight::GetBoundingBox() const
{
    return Box(position, position);
}

bool PointLight::TestRayHit(const math::Ray& ray, Float& outDistance) const
{
    RT_UNUSED(ray);
    RT_UNUSED(outDistance);

    // we assume that a ray can never hit a point light source
    return false;
}

const Color PointLight::Illuminate(const Vector4& scenePoint, RenderingContext& context, Vector4& outDirectionToLight, float& outDistance, float& outDirectPdfW) const
{
    outDirectionToLight = position - scenePoint;
    const float sqrDistance = outDirectionToLight.SqrLength3();

    outDirectPdfW = sqrDistance;
    outDistance = std::sqrt(sqrDistance);
    outDirectionToLight /= outDistance;

    return Color::SampleRGB(context.wavelength, color);
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
