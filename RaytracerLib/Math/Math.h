#pragma once

#include "../Common.h"
#include <math.h>

#include <xmmintrin.h>
#include <smmintrin.h>


#define RT_EPSILON (0.000001f)
#define RT_PI (3.14159265359f)
#define RT_2PI (6.28318530718f)
#define RT_E (2.7182818f)


namespace rt {
namespace math {

/**
 * Minimum.
 */
template<typename T>
RT_FORCE_INLINE T Min(const T a, const T b)
{
    return (a < b) ? a : b;
}

/**
 * Maximum.
 */
template<typename T>
RT_FORCE_INLINE T Max(const T a, const T b)
{
    return (a < b) ? b : a;
}

/**
 * Absolute value.
 */
template<typename T>
RT_FORCE_INLINE T Abs(const T x)
{
    if (x < static_cast<T>(0))
    {
        return -x;
    }

    return x;
}

/**
 * Returns x with sign of y
 */
RT_FORCE_INLINE float CopySignF(const float x, const float y)
{
    Bits32 xInt, yInt;
    xInt.f = x;
    yInt.f = y;
    xInt.ui = (0x7fffffff & xInt.ui) | (0x80000000 & yInt.ui);
    return xInt.f;
}

/**
 * Clamp to range.
 */
template<typename T>
RT_FORCE_INLINE T Clamp(const T x, const T min, const T max)
{
    if (x > max)
        return max;
    else if (x < min)
        return min;
    else
        return x;
}

/**
 * Linear interpolation.
 */
template<typename T>
RT_FORCE_INLINE T Lerp(const T a, const T b, const T w)
{
    return a + w * (b - a);
}

/**
 * Rounds down "x" to nearest multiply of "step"
 */
RT_FORCE_INLINE float Quantize(float x, float step)
{
    float tmp = x / step;
    tmp = floorf(tmp);
    return tmp * step;
}

/**
 * Check if a given number is NaN (not a number), according to IEEE 754 standard.
 */
RT_FORCE_INLINE bool IsNaN(float a)
{
    Bits32 num;
    num.f = a;
    return ((num.ui & 0x7F800000) == 0x7F800000) && ((num.ui & 0x7FFFFF) != 0);
}

/**
 * Check if a given number is infinity (positive or negative), according to IEEE 754 standard.
 */
RT_FORCE_INLINE bool IsInfinity(float a)
{
    Bits32 num;
    num.f = a;
    return (num.ui & 0x7FFFFFFF) == 0x7F800000;
}

/**
 * Check if a number is power of two.
 */
template<typename T>
RT_FORCE_INLINE constexpr bool PowerOfTwo(const T x)
{
    return x && !(x & (x - 1));
}


} // namespace math
} // namespace rt
