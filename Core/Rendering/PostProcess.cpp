#include "PCH.h"
#include "PostProcess.h"

namespace rt {

PostprocessParams::PostprocessParams()
    : colorFilter(math::VECTOR_ONE)
    , saturation(0.98f)
    , exposure(0.0f)
    , ditheringStrength(0.005f)
    , bloomFactor(0.0f)
{
}

bool PostprocessParams::operator == (const PostprocessParams& other) const
{
    return
        (colorFilter == other.colorFilter).All() &&
        saturation == other.saturation &&
        exposure == other.exposure &&
        ditheringStrength == other.ditheringStrength &&
        bloomFactor == other.bloomFactor;
}

bool PostprocessParams::operator != (const PostprocessParams& other) const
{
    return !(*this == other);
}

} // namespace rt
