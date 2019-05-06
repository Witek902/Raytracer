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

const RayColor ILight::GetRadiance(const RadianceParam&, float*, float*) const
{
    RT_FATAL("Cannot hit this type of light");
    return RayColor();
}

} // namespace rt
