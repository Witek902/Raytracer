#include "PCH.h"
#include "PostProcess.h"

namespace rt {

PostprocessParams::PostprocessParams()
    : colorFilter(math::VECTOR_ONE)
    , exposure(0.0f)
    , contrast(0.8f)
    , saturation(0.98f)
    , ditheringStrength(0.005f)
    , bloomFactor(0.0f)
{
}

bool PostprocessParams::operator == (const PostprocessParams& other) const
{
    return 0 == memcmp(this, &other, sizeof(PostprocessParams));
}

bool PostprocessParams::operator != (const PostprocessParams& other) const
{
    return !(*this == other);
}

} // namespace rt
