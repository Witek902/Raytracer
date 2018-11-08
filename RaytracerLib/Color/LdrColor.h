#pragma once

#include "../RayLib.h"

namespace rt {

struct LdrColor
{
    Uint8 b;
    Uint8 g;
    Uint8 r;
    Uint8 a;

    RT_FORCE_INLINE LdrColor() : r(0), g(0), b(0), a(255u) { }
    RT_FORCE_INLINE LdrColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255u) : r(r), g(g), b(b), a(a) { }
};

LdrColor Lerp(const LdrColor& colorA, const LdrColor& colorB, Uint8 factor)
{
    LdrColor result;
    result.b = static_cast<Uint8>((colorA.b * (Uint32)(255 - factor) + colorB.b * (Uint32)factor) / 256u);
    result.g = static_cast<Uint8>((colorA.g * (Uint32)(255 - factor) + colorB.g * (Uint32)factor) / 256u);
    result.r = static_cast<Uint8>((colorA.r * (Uint32)(255 - factor) + colorB.r * (Uint32)factor) / 256u);
    result.a = static_cast<Uint8>((colorA.a * (Uint32)(255 - factor) + colorB.a * (Uint32)factor) / 256u);
    return result;
}

} // namespace rt
