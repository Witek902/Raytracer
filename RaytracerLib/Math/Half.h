#pragma once

#include "Math.h"

namespace rt {
namespace math {


RT_INLINE float ConvertHalfToFloat(const Half value)
{
#ifdef RT_USE_FP16C
    const __m128i v = _mm_cvtsi32_si128(static_cast<int>(value));
    return _mm_cvtss_f32(_mm_cvtph_ps(v));
#else // RT_USE_FP16C
    Uint32 mantissa = static_cast<Uint32>(value & 0x03FF);
    Uint32 exponent = (value & 0x7C00);

    if (exponent == 0x7C00) // INF/NAN
    {
        exponent = 0x8f;
    }
    else if (exponent != 0) // The value is normalized
    {
        exponent = static_cast<Uint32>((static_cast<int>(value) >> 10) & 0x1F);
    }
    else if (mantissa != 0) // The value is denormalized
    {
        exponent = 1;

        do
        {
            exponent--;
            mantissa <<= 1;
        } while ((mantissa & 0x0400) == 0);

        mantissa &= 0x03FF;
    }
    else // The value is zero
    {
        exponent = static_cast<Uint32>(-112);
    }

    math::Bits32 result;
    result.ui = ((static_cast<Uint32>(value) & 0x8000) << 16) | ((exponent + 112) << 23) | (mantissa << 13);
    return result.f;
#endif // RT_USE_FP16C
}


} // namespace math
} // namespace rt
