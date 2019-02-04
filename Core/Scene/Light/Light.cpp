#include "PCH.h"
#include "Light.h"

namespace rt {

using namespace math;

ILight::ILight(const Vector4 color)
{
    // TODO generic spectrum
    RT_ASSERT(color.IsValid());
    RT_ASSERT((color >= Vector4::Zero()).All());
    mColor.rgbValues = color;
}

const RayColor ILight::GetRadiance(RenderingContext&, const math::Vector4&, const math::Vector4&, Float*, Float*) const
{
    RT_FATAL("Cannot hit this type of light");
    return RayColor();
}

const math::Vector4 ILight::GetNormal(const math::Vector4&) const
{
    return math::Vector4::Zero();
}

} // namespace rt
