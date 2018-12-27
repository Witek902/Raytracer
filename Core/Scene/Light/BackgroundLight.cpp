#include "PCH.h"
#include "BackgroundLight.h"
#include "../../Rendering/Context.h"
#include "../../Rendering/ShadingData.h"
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
    Vector4 rgbColor = mColor;

    // sample environment map
    if (mTexture)
    {
        const Float theta = FastACos(Clamp(dir.y, -1.0f, 1.0f));
        const Float phi = Abs(dir.x) > FLT_EPSILON ? FastATan2(dir.z, dir.x) : 0.0f;
        const Vector4 coords(phi / (2.0f * RT_PI) + 0.5f, theta / RT_PI, 0.0f, 0.0f);

        RT_ASSERT(coords.IsValid());

        const Vector4 textureColor = mTexture->Sample(coords, SamplerDesc());
        RT_ASSERT((textureColor >= Vector4::Zero()).All());

        rgbColor *= textureColor;
    }

    return Color::SampleRGB(context.wavelength, rgbColor);
}

const Color BackgroundLight::Illuminate(IlluminateParam& param) const
{
    const Vector4 randomDirLocalSpace = param.context.randomGenerator.GetHemishpere();
    param.outDirectionToLight = param.shadingData.LocalToWorld(randomDirLocalSpace);
    param.outDirectPdfW = RT_INV_PI / 2.0f; // hemisphere area
    param.outDistance = g_backgroundLightDistance;

    return GetBackgroundColor(param.outDirectionToLight, param.context);
}

const Color BackgroundLight::GetRadiance(RenderingContext& context, const math::Vector4& rayDirection, const math::Vector4& hitPoint, Float* outDirectPdfA) const
{
    RT_UNUSED(hitPoint);

    if (outDirectPdfA)
    {
        *outDirectPdfA = RT_INV_PI / 2.0f; // hemisphere area
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
