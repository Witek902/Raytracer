#include "PCH.h"
#include "BackgroundLight.h"
#include "../../Rendering/Context.h"
#include "../../Rendering/ShadingData.h"
#include "../../Textures/Texture.h"
#include "../../Math/Transcendental.h"
#include "../../Math/Geometry.h"
#include "../../Math/SamplingHelpers.h"
#include "../Camera.h"

namespace rt {

using namespace math;

// TODO this should be calculated
static const float SceneRadius = 30.0f; // TODO

BackgroundLight::BackgroundLight() = default;

BackgroundLight::BackgroundLight(const math::Vector4& color)
    : ILight(color)
{}

ILight::Type BackgroundLight::GetType() const
{
    return Type::Background;
}

const Box BackgroundLight::GetBoundingBox() const
{
    return Box::Full();
}

bool BackgroundLight::TestRayHit(const math::Ray& ray, float& outDistance) const
{
    RT_FATAL("No ray will hit background light");

    RT_UNUSED(ray);

    // any ray can hit background
    outDistance = BackgroundLightDistance;
    return true;
}

const RayColor BackgroundLight::GetBackgroundColor(const Vector4& dir, const Wavelength& wavelength) const
{
    Spectrum color = GetColor();

    // sample environment map
    if (mTexture)
    {
        const Vector4 coords = CartesianToSphericalCoordinates(dir);
        RT_ASSERT(coords.IsValid());

        const Vector4 textureColor = Vector4::Max(Vector4::Zero(), mTexture->Evaluate(coords));

        color.rgbValues *= textureColor;
    }

    return RayColor::Resolve(wavelength, color);
}

const RayColor BackgroundLight::Illuminate(const IlluminateParam& param, IlluminateResult& outResult) const
{
    const Vector4 randomDirLocalSpace = SamplingHelpers::GetHemishpere(param.sample);
    outResult.directionToLight = param.intersection.LocalToWorld(randomDirLocalSpace);
    outResult.directPdfW = UniformHemispherePdf();
    outResult.emissionPdfW = UniformSpherePdf() * UniformCirclePdf(SceneRadius);
    outResult.distance = BackgroundLightDistance;
    outResult.cosAtLight = 1.0f;

    // TODO include light rotation
    return GetBackgroundColor(outResult.directionToLight, param.wavelength);
}

const RayColor BackgroundLight::GetRadiance(const RadianceParam& param, float* outDirectPdfA, float* outEmissionPdfW) const
{
    if (outDirectPdfA)
    {
        *outDirectPdfA = UniformHemispherePdf();
    }

    if (outEmissionPdfW)
    {
        *outEmissionPdfW = UniformSpherePdf() * UniformCirclePdf(SceneRadius);
    }

    // TODO include light rotation
    return GetBackgroundColor(param.ray.dir, param.context.wavelength);
}

const RayColor BackgroundLight::Emit(const EmitParam& param, EmitResult& outResult) const
{
    // generate random direction on sphere
    // TODO texture importance sampling
    outResult.direction = SamplingHelpers::GetSphere(param.directionSample);

    // generate random origin
    const Vector4 uv = SamplingHelpers::GetCircle(param.positionSample);
    {
        Vector4 u, v;
        BuildOrthonormalBasis(outResult.direction, u, v);

        outResult.position = SceneRadius * (u * uv.x + v * uv.y - outResult.direction);
    }

    outResult.directPdfA = UniformHemispherePdf();
    outResult.emissionPdfW = UniformSpherePdf() * UniformCirclePdf(SceneRadius);
    outResult.cosAtLight = 1.0f;

    // TODO include light rotation
    return GetBackgroundColor(-outResult.direction, param.wavelength);
}

ILight::Flags BackgroundLight::GetFlags() const
{
    return Flag_None;
}

} // namespace rt
