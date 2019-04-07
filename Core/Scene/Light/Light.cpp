#include "PCH.h"
#include "Light.h"

namespace rt {

using namespace math;

ILight::ILight(const Vector4& color)
{
    // TODO generic spectrum
    RT_ASSERT(color.IsValid());
    RT_ASSERT((color >= Vector4::Zero()).All());
    mColor.rgbValues = color;
}

void ILight::SetColor(const Spectrum& color)
{
    RT_ASSERT((color.rgbValues >= Vector4::Zero()).All(), "Invalid color");
    RT_ASSERT(color.rgbValues.IsValid(), "Invalid color");
    mColor = color;
}

const RayColor ILight::GetRadiance(RenderingContext&, const math::Ray&, const math::Vector4&, float*, float*) const
{
    RT_FATAL("Cannot hit this type of light");
    return RayColor();
}

const math::Vector4 ILight::GetNormal(const math::Vector4&) const
{
    return math::Vector4::Zero();
}

} // namespace rt
