#include "PCH.h"
#include "DirectionalLight.h"
#include "../../Rendering/Context.h"

namespace rt {

using namespace math;

DirectionalLight::DirectionalLight(const math::Vector4& direction, const math::Vector4& color)
    : ILight(color)
    , mDirection(direction.Normalized3())
{
    RT_ASSERT(mDirection.IsValid());
    RT_ASSERT(Abs(mDirection.SqrLength3() - 1.0f) < 0.001f);
}

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

const Color DirectionalLight::Illuminate(IlluminateParam& param) const
{
    param.outDirectionToLight = -mDirection;
    param.outDistance = FLT_MAX;
    param.outDirectPdfW = 1.0f;

    return Color::SampleRGB(param.context.wavelength, mColor);
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
