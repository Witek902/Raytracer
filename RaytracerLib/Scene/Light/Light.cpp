#include "PCH.h"
#include "Light.h"

namespace rt {

using namespace math;

ILight::ILight(const Vector4 color)
    : mColor(color)
{
    RT_ASSERT(mColor.IsValid());
    RT_ASSERT(mColor >= Vector4::Zero());
}

} // namespace rt
