#pragma once

#include "Half.h"

namespace rt {
namespace math {


static_assert(sizeof(Half) == 2, "Invalid Half type size");
static_assert(sizeof(Half2) == 4, "Invalid Half2 type size");
static_assert(sizeof(Half3) == 6, "Invalid Half3 type size");
static_assert(sizeof(Half4) == 8, "Invalid Half4 type size");


Half::Half(const float other)
{
#ifdef RT_USE_FP16C
    __m128 v1 = _mm_set_ss(other);
    __m128i v2 = _mm_cvtps_ph(v1, 0);
    value = static_cast<Uint16>(_mm_cvtsi128_si32(v2));
#else
    Uint32 result;

    Uint32 iValue = *reinterpret_cast<const Uint32*>(&other);
    Uint32 sign = (iValue & 0x80000000U) >> 16U;
    iValue = iValue & 0x7FFFFFFFU;

    if (iValue > 0x477FE000U)
    {
        // The number is too large to be represented as a half. Saturate to infinity.
        if (((iValue & 0x7F800000) == 0x7F800000) && ((iValue & 0x7FFFFF) != 0))
        {
            result = 0x7FFF; // NAN
        }
        else
        {
            result = 0x7C00U; // INF
        }
    }
    else if (!iValue)
    {
        result = 0;
    }
    else
    {
        if (iValue < 0x38800000U)
        {
            // The number is too small to be represented as a normalized half.
            // Convert it to a denormalized value.
            Uint32 Shift = 113U - (iValue >> 23U);
            iValue = (0x800000U | (iValue & 0x7FFFFFU)) >> Shift;
        }
        else
        {
            // Rebias the exponent to represent the value as a normalized half.
            iValue += 0xC8000000U;
        }

        result = ((iValue + 0x0FFFU + ((iValue >> 13U) & 1U)) >> 13U) & 0x7FFFU;
    }

    value = static_cast<Uint16>(result | sign);
#endif // !_XM_F16C_INTRINSICS_
}

float Half::ToFloat() const
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
