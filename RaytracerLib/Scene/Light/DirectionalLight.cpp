#include "PCH.h"
#include "DirectionalLight.h"
#include "../../Rendering/Context.h"

namespace rt {

using namespace math;

const Box DirectionalLight::GetBoundingBox() const
{
    return Box::Empty();
}

bool DirectionalLight::TestRayHit(const math::Ray& ray, Float& outDistance) const
{
    RT_UNUSED(ray);
    RT_UNUSED(outDistance);

    // we assume that a ray can never hit a directional light source
    return false;
}

const Color DirectionalLight::Illuminate(const Vector4& scenePoint, RenderingContext& context, Vector4& outDirectionToLight, float& outDistance, float& outDirectPdfW) const
{
    RT_UNUSED(context);
    RT_UNUSED(scenePoint);

    outDirectionToLight = -direction;
    outDistance = 1.0f;
    outDirectPdfW = 1.0f;

    return Color::SampleRGB(context.wavelength, color);
}

const Color DirectionalLight::GetRadiance(RenderingContext& context, const math::Vector4& rayDirection, const math::Vector4& hitPoint, Float* outDirectPdfA) const
{
    RT_UNUSED(rayDirection);
    RT_UNUSED(hitPoint);
    RT_UNUSED(context);
    RT_UNUSED(outDirectPdfA);

    RT_FATAL("Cannot hit directinal");
    return Color();
}

bool DirectionalLight::IsFinite() const
{
    return false;
}

bool DirectionalLight::IsDelta() const
{
    return true;
}

} // namespace rt
