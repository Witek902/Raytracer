#include "PCH.h"
#include "Light.h"

namespace rt {

using namespace math;

ILight::ILight(const Vector4 color)
    : mColor(color)
{
    RT_ASSERT(mColor.IsValid());
    RT_ASSERT((mColor >= Vector4::Zero()).All());
}

const Color ILight::GetRadiance(RenderingContext&, const math::Vector4&, const math::Vector4&, Float*) const
{
    RT_FATAL("Cannot hit this type of light");
    return Color();
}

const math::Vector4 ILight::GetNormal(const math::Vector4&) const
{
    RT_FATAL("Cannot hit this type of light");
    return math::Vector4::Zero();
}

} // namespace rt
