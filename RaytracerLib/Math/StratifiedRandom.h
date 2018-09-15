#pragma once

#include "../RayLib.h"
#include "Float2.h"

namespace rt {
namespace math {

class Random;

class RAYLIB_API StratifiedRandom
{
public:
    StratifiedRandom(Uint32 numStrata = 2);

    void NextStratum();

    Float2 GetFloat2(Random& random) const;

private:
    Uint32 stratumIndex;
    Uint32 numStrata;

    Float2 offset;
    Float2 scale;
};


} // namespace Math
} // namespace NFE
