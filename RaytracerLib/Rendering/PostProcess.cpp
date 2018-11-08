#include "PCH.h"
#include "PostProcess.h"

namespace rt {

bool PostprocessParams::operator == (const PostprocessParams& other) const
{
    return
        colorFilter == other.colorFilter &&
        exposure == other.exposure &&
        ditheringStrength == other.ditheringStrength;    
}

bool PostprocessParams::operator != (const PostprocessParams& other) const
{
    return !(*this == other);
}

} // namespace rt
