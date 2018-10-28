#include "PCH.h"
#include "BackgroundLight.h"
#include "../../Rendering/Context.h"

namespace rt {

using namespace math;

static const Float g_backgroundLightDistance = 1.0e+36f;

const Box BackgroundLight::GetBoundingBox() const
{
    return Box::Full();
}

bool BackgroundLight::TestRayHit(const math::Ray& ray, Float& outDistance) const
{
    RT_UNUSED(ray);

    // any ray can hit background
    outDistance = g_backgroundLightDistance;
    return true;
}

const Color BackgroundLight::Illuminate(const Vector4& scenePoint, RenderingContext& context, Vector4& outDirectionToLight, float& outDistance, float& outDirectPdfW) const
{
    RT_UNUSED(scenePoint);

    outDirectionToLight = context.randomGenerator.GetSphere();
    outDirectPdfW = RT_INV_PI / 4.0f;
    outDistance = g_backgroundLightDistance;

    return Color::SampleRGB(context.wavelength, color);
}

const Color BackgroundLight::GetRadiance(RenderingContext& context, const math::Vector4& rayDirection, const math::Vector4& hitPoint, Float* outDirectPdfA) const
{
    RT_UNUSED(rayDirection);
    RT_UNUSED(hitPoint);
    RT_UNUSED(context);
    
    if (outDirectPdfA)
    {
        *outDirectPdfA = RT_INV_PI / 4.0f;
    }

    return Color::SampleRGB(context.wavelength, color);
}

bool BackgroundLight::IsFinite() const
{
    return false;
}

bool BackgroundLight::IsDelta() const
{
    return false;
}

} // namespace rt
