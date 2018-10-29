#include "PCH.h"
#include "BackgroundLight.h"
#include "../../Rendering/Context.h"
#include "../../Utils/Bitmap.h"
#include "../../Math/Transcendental.h"

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

const Color BackgroundLight::GetBackgroundColor(const Vector4& dir, RenderingContext& context) const
{
    Vector4 rgbColor = color;

    // sample environment map
    if (mTexture)
    {
        const Float theta = ACos(dir.y);
        const Float phi = FastATan2(dir.z, dir.x);
        const Vector4 coords(phi / (2.0f * RT_PI) + 0.5f, theta / RT_PI, 0.0f, 0.0f);

        rgbColor *= mTexture->Sample(coords, SamplerDesc());
    }

    return Color::SampleRGB(context.wavelength, rgbColor);
}

const Color BackgroundLight::Illuminate(const Vector4& scenePoint, RenderingContext& context, Vector4& outDirectionToLight, float& outDistance, float& outDirectPdfW) const
{
    RT_UNUSED(scenePoint);

    outDirectionToLight = context.randomGenerator.GetSphere();
    outDirectPdfW = RT_INV_PI / 4.0f;
    outDistance = g_backgroundLightDistance;

    return GetBackgroundColor(outDirectionToLight, context);
}

const Color BackgroundLight::GetRadiance(RenderingContext& context, const math::Vector4& rayDirection, const math::Vector4& hitPoint, Float* outDirectPdfA) const
{
    RT_UNUSED(hitPoint);
    
    if (outDirectPdfA)
    {
        *outDirectPdfA = RT_INV_PI / 4.0f;
    }

    return GetBackgroundColor(rayDirection, context);
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
