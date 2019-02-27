#include "PCH.h"
#include "BackgroundLight.h"
#include "../../Rendering/Context.h"
#include "../../Rendering/ShadingData.h"
#include "../../Utils/Bitmap.h"
#include "../../Math/Transcendental.h"
#include "../../Math/Geometry.h"
#include "../../Math/SamplingHelpers.h"
#include "../Camera.h"

namespace rt {

using namespace math;

// TODO this should be calculated
static const float SceneRadius = 5.8f;

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

const RayColor BackgroundLight::GetBackgroundColor(const Vector4& dir, const Wavelength& wavelength) const
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

    return RayColor::Resolve(wavelength, color);
}

const RayColor BackgroundLight::Illuminate(const IlluminateParam& param, IlluminateResult& outResult) const
{
    const Vector4 randomDirLocalSpace = SamplingHelpers::GetHemishpere(param.sample);
    outResult.directionToLight = param.shadingData.LocalToWorld(randomDirLocalSpace);
    outResult.directPdfW = UniformHemispherePdf();
    outResult.emissionPdfW = UniformSpherePdf() * UniformCirclePdf(SceneRadius);
    outResult.distance = BackgroundLightDistance;
    outResult.cosAtLight = 1.0f;

    return GetBackgroundColor(outResult.directionToLight, param.wavelength);
}

const RayColor BackgroundLight::GetRadiance(RenderingContext& context, const math::Vector4& rayDirection, const math::Vector4& hitPoint, float* outDirectPdfA, float* outEmissionPdfW) const
{
    RT_UNUSED(hitPoint);

    if (outDirectPdfA)
    {
        *outDirectPdfA = UniformHemispherePdf();
    }

    if (outEmissionPdfW)
    {
        *outEmissionPdfW = UniformSpherePdf() * UniformCirclePdf(SceneRadius);
    }

    return GetBackgroundColor(rayDirection, context.wavelength);
}

const RayColor BackgroundLight::Emit(const EmitParam& param, EmitResult& outResult) const
{
    // generate random direction on sphere
    // TODO texture importance sampling
    outResult.direction = SamplingHelpers::GetSphere(param.sample);

    // generate random origin
    const Vector4 uv = SamplingHelpers::GetCircle(param.sample2);
    {
        Vector4 u, v;
        BuildOrthonormalBasis(outResult.direction, u, v);

        outResult.position = SceneRadius * (u * uv.x + v * uv.y - outResult.direction);
    }

    outResult.directPdfA = UniformHemispherePdf();
    outResult.emissionPdfW = UniformSpherePdf() * UniformCirclePdf(SceneRadius);
    outResult.cosAtLight = 1.0f;

    return GetBackgroundColor(-outResult.direction, param.wavelength);
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
