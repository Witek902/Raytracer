#pragma once

#include "../RayLib.h"

#include <math.h>

#ifdef RT_USE_SSE
#include <xmmintrin.h>
#include <smmintrin.h>
#include <immintrin.h>
#include <emmintrin.h>
#endif // RT_USE_SSE

#define RT_EPSILON (0.000001f)
#define RT_PI (3.14159265359f)
#define RT_SQRT_PI (1.77245385091f)
#define RT_INV_PI (0.31830988618f)
#define RT_2PI (6.28318530718f)
#define RT_E (2.7182818f)


namespace rt {
namespace math {


// set/check flushing denormalized floats to zero
RAYLIB_API void SetFlushDenormalsToZero(bool enable = true);
RAYLIB_API bool GetFlushDenormalsToZero();


// Union providing easy manipulations on 32-bit values
union Bits32
{
    float f;
    uint32 ui;
    int32 si;
};

// Union providing easy manipulations on 64-bit values
union Bits64
{
    double f;
    uint64 ui;
    int64 si;
};

// convert degrees to radians
RT_FORCE_INLINE constexpr float DegToRad(const float x)
{
    return x / 180.0f * RT_PI;
}

// convert radians to degrees
RT_FORCE_INLINE constexpr float RadToDeg(const float x)
{
    return x / RT_PI * 180.0f;
}

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
RT_FORCE_INLINE constexpr const T Max(const T a, const T b)
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
RT_FORCE_INLINE constexpr const T Abs(const T x)
{
    static_assert(!std::is_integral<T>::value || std::is_signed<T>::value, "Abs() must be used with signed types");

    if (x < static_cast<T>(0))
    {
        return -x;
    }

    return x;
}

// compute floor and convert to integer
RT_FORCE_INLINE constexpr int32 FloorInt(float fp)
{
    const int32 i = static_cast<int32>(fp);
    return fp < i ? (i - 1) : i;
}

// x^2
template<typename T>
RT_FORCE_INLINE constexpr const T Sqr(const T x)
{
    return x * x;
}

// x^3
template<typename T>
RT_FORCE_INLINE constexpr const T Cube(const T x)
{
    return x * x * x;
}

RT_FORCE_INLINE static float FastDivide(float a, float b)
{
#ifdef RT_USE_SSE
    return a * _mm_cvtss_f32(_mm_rcp_ss(_mm_load_ss(&b)));
#else
    return a / b; // TODO
#endif
}

// Square root
RT_FORCE_INLINE float Sqrt(const float x)
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
RT_FORCE_INLINE const T Signum(const T x)
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
RT_FORCE_INLINE constexpr const T Clamp(const T x, const T min, const T max)
{
    if (x > max)
        return max;
    else if (x < min)
        return min;
    else
        return x;
}

template<typename T>
RT_FORCE_INLINE constexpr const T SmoothStep(const T x)
{
    return x * x * (T(3.0f) - x * T(2.0f));
}

template<typename T>
RT_FORCE_INLINE static constexpr const T Step(const T a, const T b, T x)
{
    return Clamp((x - a) / (b - a), T(0.0f), T(1.0f));
}


template<typename T>
RT_FORCE_INLINE static constexpr float SmoothStep(const T a, const T b, T x)
{
    return SmoothStep(Step(a, b, x));
}

// Linear interpolation.
template<typename T>
RT_FORCE_INLINE constexpr const T Lerp(const T a, const T b, const T w)
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

// Roundup to next power of two
RT_FORCE_INLINE constexpr uint32 NextPowerOfTwo(uint32 v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
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
RT_FORCE_INLINE constexpr uint32 Hash(uint32 a)
{
    a = (a ^ 61) ^ (a >> 16);
    a += (a << 3);
    a ^= (a >> 4);
    a *= 0x27d4eb2d;
    a ^= (a >> 15);
    return a;
}

// MurmurHash3
RT_FORCE_INLINE constexpr uint64 Hash(uint64 h)
{
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccdL;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53L;
    h ^= h >> 33;
    return h;
}

// bit population count
RT_FORCE_INLINE uint32 PopCount(uint32 x)
{
#if defined(WIN32) && RT_USE_BMI
    return __popcnt(x);
#elif defined(__LINUX__) | defined(__linux__)
    return __builtin_popcount(x);
#else
    // https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
    x = x - ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    return ((x + (x >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
#endif // defined(WIN32)
}

} // namespace math
} // namespace rt
