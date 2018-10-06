#include "PCH.h"
#include "StratifiedRandom.h"
#include "Math.h"
#include "Random.h"

namespace rt {
namespace math {

StratifiedRandom::StratifiedRandom(Uint32 numStrata)
    : stratumIndex(0)
    , numStrata(numStrata)
{
    scale = Float2(1.0f / (float)numStrata, 1.0f / (float)numStrata);
}

void StratifiedRandom::NextStratum()
{
    stratumIndex++;

    Uint32 stratumX, stratumY;
    DecodeMorton(stratumIndex, stratumX, stratumY);
    stratumX %= numStrata;
    stratumY %= numStrata;
    offset = Float2((float)stratumX, (float)stratumY);
}

Float2 StratifiedRandom::GetFloat2(Random& random) const
{
    return (offset + random.GetFloat2()) * scale;
}

} // namespace Math
} // namespace NFE
