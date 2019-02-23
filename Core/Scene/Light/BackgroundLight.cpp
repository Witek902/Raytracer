#include "PCH.h"
#include "BackgroundLight.h"
#include "../../Rendering/Context.h"
#include "../../Rendering/ShadingData.h"
#include "../../Utils/Bitmap.h"
#include "../../Math/Transcendental.h"

namespace rt {

using namespace math;

const Box BackgroundLight::GetBoundingBox() const
{
    return Box::Full();
}

bool BackgroundLight::TestRayHit(const math::Ray& ray, float& outDistance) const
{
    RT_UNUSED(ray);

    // any ray can hit background
    outDistance = BackgroundLightDistance;
    return true;
}

const RayColor BackgroundLight::GetBackgroundColor(const Vector4& dir, RenderingContext& context) const
{
    Spectrum color = mColor;

    // sample environment map
    if (mTexture)
    {
        const float theta = FastACos(Clamp(dir.y, -1.0f, 1.0f));
        const float phi = Abs(dir.x) > FLT_EPSILON ? FastATan2(dir.z, dir.x) : 0.0f;
        const Vector4 coords(phi / (2.0f * RT_PI) + 0.5f, theta / RT_PI, 0.0f, 0.0f);

        RT_ASSERT(coords.IsValid());

        const Vector4 textureColor = mTexture->Evaluate(coords, SamplerDesc());
        RT_ASSERT((textureColor >= Vector4::Zero()).All());

        color.rgbValues *= textureColor;
    }

    return RayColor::Resolve(context.wavelength, color);
}

const RayColor BackgroundLight::Illuminate(IlluminateParam& param) const
{
    const Vector4 randomDirLocalSpace = param.context.randomGenerator.GetHemishpere();
    param.outDirectionToLight = param.shadingData.LocalToWorld(randomDirLocalSpace);
    param.outDirectPdfW = RT_INV_PI / 2.0f; // hemisphere area
    param.outEmissionPdfW = 0.0f;
    param.outDistance = BackgroundLightDistance;
    param.outCosAtLight = 1.0f;

    return GetBackgroundColor(param.outDirectionToLight, param.context);
}

const RayColor BackgroundLight::GetRadiance(RenderingContext& context, const math::Vector4& rayDirection, const math::Vector4& hitPoint, float* outDirectPdfA, float* outEmissionPdfW) const
{
    RT_UNUSED(hitPoint);

    if (outDirectPdfA)
    {
        *outDirectPdfA = RT_INV_PI / 2.0f; // hemisphere area
    }

    if (outEmissionPdfW)
    {
        *outEmissionPdfW = 0.0f;
    }

    return GetBackgroundColor(rayDirection, context);
}

const RayColor BackgroundLight::Emit(RenderingContext& ctx, EmitResult& outResult) const
{
    // TODO how to generate ray for background light?

    RT_UNUSED(ctx);

    outResult.emissionPdfW = 0.0f;
    outResult.directPdfA = RT_INV_PI / 2.0f; // hemisphere area
    outResult.cosAtLight = 1.0f;

    return RayColor::Zero();
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
