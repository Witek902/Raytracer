#pragma once

#include "../RayLib.h"

#include <math.h>
#include <xmmintrin.h>
#include <smmintrin.h>
#include <immintrin.h>
#include <emmintrin.h>
#ifdef _MSC_VER
#include <intrin.h>
#endif // _MSC_VER

#define RT_EPSILON (0.000001f)
#define RT_PI (3.14159265359f)
#define RT_INV_PI (0.31830988618f)
#define RT_2PI (6.28318530718f)
#define RT_E (2.7182818f)


namespace rt {
namespace math {


// set/check flushing denormalized floats to zero
RAYLIB_API void SetFlushDenormalsToZero(bool enable = true);
RAYLIB_API bool GetFlushDenormalsToZero();

// half (16-bit) floating point type
using Half = Uint16;

// Union providing easy manipulations on 32-bit values
union Bits32
{
    float f;
    Uint32 ui;
    Int32 si;
};

// Union providing easy manipulations on 64-bit values
union Bits64
{
    double f;
    Uint64 ui;
    Int64 si;
};

// Minimum
template<typename T>
RT_FORCE_INLINE constexpr const T Min(const T a, const T b)
{
    return (a < b) ? a : b;
}

template<typename T, typename ... Types>
RT_FORCE_INLINE constexpr const T Min(const T a, const T b, const Types ... r)
{
    return Min(a < b ? a : b, r ...);
}

// Maximum.
template<typename T>
RT_FORCE_INLINE constexpr T Max(const T a, const T b)
{
    return (a < b) ? b : a;
}

template<typename T, typename ... Types>
RT_FORCE_INLINE constexpr const T Max(const T a, const T b, const Types ... r)
{
    return Max(a > b ? a : b, r ...);
}

// Absolute value.
template<typename T>
RT_FORCE_INLINE constexpr T Abs(const T x)
{
    if (x < static_cast<T>(0))
    {
        return -x;
    }

    return x;
}

// x^2
template<typename T>
RT_FORCE_INLINE constexpr T Sqr(const T x)
{
    return x * x;
}

// x^3
template<typename T>
RT_FORCE_INLINE constexpr T Cube(const T x)
{
    return x * x * x;
}

// Square root
RT_FORCE_INLINE Float Sqrt(const Float x)
{
    RT_ASSERT(x >= 0.0f, "Invalid argument");
    return sqrtf(x);
}

// Returns x with sign of y
RT_FORCE_INLINE float CopySign(const float x, const float y)
{
    Bits32 xInt, yInt;
    xInt.f = x;
    yInt.f = y;
    xInt.ui = (0x7fffffff & xInt.ui) | (0x80000000 & yInt.ui);
    return xInt.f;
}

template<typename T>
RT_FORCE_INLINE T Signum(const T x)
{
    if (x > T(0))
    {
        return T(1);
    }
    else if (x < T(0))
    {
        return T(-1);
    }
    else
    {
        return T(0);
    }
}

// Clamp to range.
template<typename T>
RT_FORCE_INLINE constexpr T Clamp(const T x, const T min, const T max)
{
    if (x > max)
        return max;
    else if (x < min)
        return min;
    else
        return x;
}

// Linear interpolation.
template<typename T>
RT_FORCE_INLINE constexpr T Lerp(const T a, const T b, const T w)
{
    return a + w * (b - a);
}

// Check if a given number is NaN (not a number), according to IEEE 754 standard.
RT_FORCE_INLINE bool IsNaN(float x)
{
    Bits32 num;
    num.f = x;
    return ((num.ui & 0x7F800000) == 0x7F800000) && ((num.ui & 0x7FFFFF) != 0);
}

// Check if a given number is infinity (positive or negative), according to IEEE 754 standard.
RT_FORCE_INLINE bool IsInfinity(float x)
{
    Bits32 num;
    num.f = x;
    return (num.ui & 0x7FFFFFFF) == 0x7F800000;
}

// Check if a given number is not NaN nor infinity
RT_FORCE_INLINE bool IsValid(float x)
{
    Bits32 num;
    num.f = x;
    return (num.ui & 0x7F800000) != 0x7F800000;
}

// Check if a number is power of two.
template<typename T>
RT_FORCE_INLINE constexpr bool PowerOfTwo(const T x)
{
    static_assert(std::is_unsigned<T>::value, "Unsiged type expected");

    return x && !(x & (x - 1));
}

template<typename T>
RT_FORCE_INLINE constexpr const T RoundUp(const T x, const T multiple)
{
    T remainder = x % multiple;
    if (remainder == 0)
        return x;

    return x + multiple - remainder;
}

// Wang hash
RT_FORCE_INLINE constexpr Uint32 Hash(Uint32 a)
{
    a = (a ^ 61) ^ (a >> 16);
    a += (a << 3);
    a ^= (a >> 4);
    a *= 0x27d4eb2d;
    a ^= (a >> 15);
    return a;
}

// MurmurHash3
RT_FORCE_INLINE constexpr Uint64 Hash(Uint64 h)
{
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccdL;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53L;
    h ^= h >> 33;
    return h;
}

// bit population count
RT_FORCE_INLINE Uint32 PopCount(Uint32 x)
{
#if defined(WIN32)
    return __popcnt(x);
#elif defined(__LINUX__) | defined(__linux__)
    return __builtin_popcount(x);
#endif // defined(WIN32)
}

} // namespace math
} // namespace rt
