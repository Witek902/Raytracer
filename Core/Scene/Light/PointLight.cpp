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

bool PointLight::TestRayHit(const math::Ray& ray, float& outDistance) const
{
    RT_UNUSED(ray);
    RT_UNUSED(outDistance);

    // we assume that a ray can never hit a point light source
    return false;
}

const RayColor PointLight::Illuminate(IlluminateParam& param) const
{
    param.outDirectionToLight = mPosition - param.shadingData.frame.GetTranslation();
    const float sqrDistance = param.outDirectionToLight.SqrLength3();

    param.outDirectPdfW = sqrDistance;
    param.outEmissionPdfW = RT_INV_PI / 4.0f;
    param.outDistance = std::sqrt(sqrDistance);
    param.outDirectionToLight /= param.outDistance;
    param.outCosAtLight = 1.0f;

    return RayColor::Resolve(param.context.wavelength, mColor);
}

const RayColor PointLight::Emit(RenderingContext& ctx, EmitResult& outResult) const
{
    outResult.position = mPosition;
    outResult.direction = ctx.randomGenerator.GetSphere();
    outResult.emissionPdfW = RT_INV_PI / 4.0f;
    outResult.directPdfA = 1.0f;
    outResult.cosAtLight = 1.0f;

    // TODO texture
    return RayColor::Resolve(ctx.wavelength, mColor);
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
