#pragma once

#include "../RayLib.h"

namespace rt {

struct LdrColor
{
    uint8 b;
    uint8 g;
    uint8 r;
    uint8 a;

    RT_FORCE_INLINE LdrColor() : b(0), g(0), r(0), a(255u) { }
    RT_FORCE_INLINE LdrColor(uint8 r, uint8 g, uint8 b, uint8 a = 255u) : b(b), g(g), r(r), a(a) { }
};

LdrColor Lerp(const LdrColor& colorA, const LdrColor& colorB, uint8 factor)
{
    LdrColor result;
    result.b = static_cast<uint8>((colorA.b * (uint32)(255 - factor) + colorB.b * (uint32)factor) / 256u);
    result.g = static_cast<uint8>((colorA.g * (uint32)(255 - factor) + colorB.g * (uint32)factor) / 256u);
    result.r = static_cast<uint8>((colorA.r * (uint32)(255 - factor) + colorB.r * (uint32)factor) / 256u);
    result.a = static_cast<uint8>((colorA.a * (uint32)(255 - factor) + colorB.a * (uint32)factor) / 256u);
    return result;
}

} // namespace rt
