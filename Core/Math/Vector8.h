#pragma once

#include "Math.h"
#include "Vector4.h"
#include "VectorBool8.h"

namespace rt {
namespace math {

/**
 * 8-element SIMD vector.
 */
struct RT_ALIGN(32) Vector8
{
    // constructors
    RT_FORCE_INLINE Vector8() = default;
    RT_FORCE_INLINE Vector8(const Vector8& other);
    RT_FORCE_INLINE static const Vector8 Zero();
    RT_FORCE_INLINE Vector8(const __m256& m);
    RT_FORCE_INLINE explicit Vector8(const Vector4& lo);
    RT_FORCE_INLINE Vector8(const Vector4& lo, const Vector4& hi);
    RT_FORCE_INLINE explicit Vector8(const float scalar);
    RT_FORCE_INLINE explicit Vector8(const Int32 scalar);
    RT_FORCE_INLINE explicit Vector8(const Uint32 scalar);
    RT_FORCE_INLINE Vector8(float e0, float e1, float e2, float e3, float e4, float e5, float e6, float e7);
    RT_FORCE_INLINE Vector8(Int32 e0, Int32 e1, Int32 e2, Int32 e3, Int32 e4, Int32 e5, Int32 e6, Int32 e7);
    RT_FORCE_INLINE Vector8(Uint32 e0, Uint32 e1, Uint32 e2, Uint32 e3, Uint32 e4, Uint32 e5, Uint32 e6, Uint32 e7);
    RT_FORCE_INLINE Vector8(const float* src);
    RT_FORCE_INLINE Vector8& operator = (const Vector8& other);
    RT_FORCE_INLINE static const Vector8 FromInteger(Int32 x);

    // Rearrange vector elements (in both lanes, parallel)
    template<Uint32 ix = 0, Uint32 iy = 1, Uint32 iz = 2, Uint32 iw = 3>
    RT_FORCE_INLINE const Vector8 Swizzle() const;

    RT_FORCE_INLINE operator __m256() const { return v; }
    RT_FORCE_INLINE operator __m256i() const { return reinterpret_cast<const __m256i*>(&v)[0]; }
    RT_FORCE_INLINE float operator[] (Uint32 index) const { return f[index]; }
    RT_FORCE_INLINE float& operator[] (Uint32 index) { return f[index]; }

    // extract lower lanes
    RT_FORCE_INLINE const Vector4 Low() const
    {
        return Vector4(_mm256_extractf128_ps(v, 0));
    }

    // extract higher lanes
    RT_FORCE_INLINE const Vector4 High() const
    {
        return Vector4(_mm256_extractf128_ps(v, 1));
    }

    // simple arithmetics
    RT_FORCE_INLINE const Vector8 operator - () const;
    RT_FORCE_INLINE const Vector8 operator + (const Vector8& b) const;
    RT_FORCE_INLINE const Vector8 operator - (const Vector8& b) const;
    RT_FORCE_INLINE const Vector8 operator * (const Vector8& b) const;
    RT_FORCE_INLINE const Vector8 operator / (const Vector8& b) const;
    RT_FORCE_INLINE const Vector8 operator * (float b) const;
    RT_FORCE_INLINE const Vector8 operator / (float b) const;
    RT_FORCE_INLINE Vector8& operator += (const Vector8& b);
    RT_FORCE_INLINE Vector8& operator -= (const Vector8& b);
    RT_FORCE_INLINE Vector8& operator *= (const Vector8& b);
    RT_FORCE_INLINE Vector8& operator /= (const Vector8& b);
    RT_FORCE_INLINE Vector8& operator *= (float b);
    RT_FORCE_INLINE Vector8& operator /= (float b);

    // comparison operators (returns true, if all the elements satisfy the equation)
    RT_FORCE_INLINE const VectorBool8 operator == (const Vector8& b) const;
    RT_FORCE_INLINE const VectorBool8 operator < (const Vector8& b) const;
    RT_FORCE_INLINE const VectorBool8 operator <= (const Vector8& b) const;
    RT_FORCE_INLINE const VectorBool8 operator > (const Vector8& b) const;
    RT_FORCE_INLINE const VectorBool8 operator >= (const Vector8& b) const;
    RT_FORCE_INLINE const VectorBool8 operator != (const Vector8& b) const;

    // bitwise logic operations
    RT_FORCE_INLINE const Vector8 operator & (const Vector8& b) const;
    RT_FORCE_INLINE const Vector8 operator | (const Vector8& b) const;
    RT_FORCE_INLINE const Vector8 operator ^ (const Vector8& b) const;
    RT_FORCE_INLINE Vector8& operator &= (const Vector8& b);
    RT_FORCE_INLINE Vector8& operator |= (const Vector8& b);
    RT_FORCE_INLINE Vector8& operator ^= (const Vector8& b);

    RT_FORCE_INLINE static const Vector8 Floor(const Vector8& v);
    RT_FORCE_INLINE static const Vector8 Sqrt(const Vector8& v);
    RT_FORCE_INLINE static const Vector8 Reciprocal(const Vector8& v);
    RT_FORCE_INLINE static const Vector8 FastReciprocal(const Vector8& v);
    RT_FORCE_INLINE static const Vector8 Lerp(const Vector8& v1, const Vector8& v2, const Vector8& weight);
    RT_FORCE_INLINE static const Vector8 Lerp(const Vector8& v1, const Vector8& v2, float weight);
    RT_FORCE_INLINE static const Vector8 Min(const Vector8& a, const Vector8& b);
    RT_FORCE_INLINE static const Vector8 Max(const Vector8& a, const Vector8& b);
    RT_FORCE_INLINE static const Vector8 Abs(const Vector8& v);
    RT_FORCE_INLINE const Vector8 Clamped(const Vector8& min, const Vector8& max) const;

    // Build mask of sign bits.
    RT_FORCE_INLINE Int32 GetSignMask() const;

    // For each vector component, copy value from "a" if "sel" > 0.0f, or from "b" otherwise.
    RT_FORCE_INLINE static const Vector8 Select(const Vector8& a, const Vector8& b, const VectorBool8& sel);

    // Check if the vector is equal to zero
    RT_FORCE_INLINE bool IsZero() const;

    // Check if any component is NaN
    RT_FORCE_INLINE bool IsNaN() const;

    // Check if any component is an infinity
    RT_FORCE_INLINE bool IsInfinite() const;

    // Check if is not NaN or infinity
    RT_FORCE_INLINE bool IsValid() const;

    // Check if two vectors are (almost) equal.
    RT_FORCE_INLINE static bool AlmostEqual(const Vector8& v1, const Vector8& v2, float epsilon = RT_EPSILON);

    // Fused multiply and add (a * b + c)
    RT_FORCE_INLINE static const Vector8 MulAndAdd(const Vector8& a, const Vector8& b, const Vector8& c);
    RT_FORCE_INLINE static const Vector8 MulAndAdd(const Vector8& a, const float b, const Vector8& c);

    // Fused multiply and subtract (a * b - c)
    RT_FORCE_INLINE static const Vector8 MulAndSub(const Vector8& a, const Vector8& b, const Vector8& c);
    RT_FORCE_INLINE static const Vector8 MulAndSub(const Vector8& a, const float b, const Vector8& c);

    // Fused multiply (negated) and add (a * b + c)
    // Fused multiply (negated) and add (a * b + c)
    RT_FORCE_INLINE static const Vector8 NegMulAndAdd(const Vector8& a, const Vector8& b, const Vector8& c);
    RT_FORCE_INLINE static const Vector8 NegMulAndAdd(const Vector8& a, const float b, const Vector8& c);

    // Fused multiply (negated) and subtract (a * b - c)
    RT_FORCE_INLINE static const Vector8 NegMulAndSub(const Vector8& a, const Vector8& b, const Vector8& c);
    RT_FORCE_INLINE static const Vector8 NegMulAndSub(const Vector8& a, const float b, const Vector8& c);

    // Calculate horizontal maximum. Result is splatted across all elements.
    RT_FORCE_INLINE const Vector8 HorizontalMax() const;

    // Compute fmodf(x, 1.0f)
    RT_FORCE_INLINE static const Vector8 Fmod1(const Vector8 x);

    // transpose 8x8 matrix
    RT_FORCE_INLINE static void Transpose8x8(Vector8& v0, Vector8& v1, Vector8& v2, Vector8& v3, Vector8& v4, Vector8& v5, Vector8& v6, Vector8& v7);

private:

    union
    {
        float f[8];
        Int32 i[8];
        Uint32 u[8];
        __m256 v;
    };
};

// like Vector8::operator * (float)
RT_FORCE_INLINE const Vector8 operator*(float a, const Vector8& b);

// some commonly used constants
RT_GLOBAL_CONST Vector8 VECTOR8_EPSILON = { RT_EPSILON, RT_EPSILON, RT_EPSILON, RT_EPSILON, RT_EPSILON, RT_EPSILON, RT_EPSILON, RT_EPSILON };
RT_GLOBAL_CONST Vector8 VECTOR8_HALVES = { 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f };
RT_GLOBAL_CONST Vector8 VECTOR8_MIN = { std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min() };
RT_GLOBAL_CONST Vector8 VECTOR8_MAX = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
RT_GLOBAL_CONST Vector8 VECTOR8_INF = { std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity() };
RT_GLOBAL_CONST Vector8 VECTOR8_ONE = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
RT_GLOBAL_CONST Vector8 VECTOR8_MINUS_ONE = { -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f };
RT_GLOBAL_CONST Vector8 VECTOR8_MASK_X = { 0xFFFFFFFFu, 0u, 0u, 0u, 0u, 0u, 0u, 0u };
RT_GLOBAL_CONST Vector8 VECTOR8_MASK_Y = { 0u, 0xFFFFFFFFu, 0u, 0u, 0u, 0u, 0u, 0u };
RT_GLOBAL_CONST Vector8 VECTOR8_MASK_Z = { 0u, 0u, 0xFFFFFFFF, 0u, 0u, 0u, 0u, 0u };
RT_GLOBAL_CONST Vector8 VECTOR8_MASK_W = { 0u, 0u, 0u, 0xFFFFFFFF, 0u, 0u, 0u, 0u };
RT_GLOBAL_CONST Vector8 VECTOR8_MASK_ALL = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
RT_GLOBAL_CONST Vector8 VECTOR8_MASK_ABS = { 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF };
RT_GLOBAL_CONST Vector8 VECTOR8_MASK_SIGN = { 0x80000000u, 0x80000000u, 0x80000000u, 0x80000000u, 0x80000000u, 0x80000000u, 0x80000000u, 0x80000000u };
RT_GLOBAL_CONST Vector8 VECTOR8_INV_255 = { 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f };
RT_GLOBAL_CONST Vector8 VECTOR8_255 = { 255.0f, 255.0f, 255.0f, 255.0f, 255.0f, 255.0f, 255.0f, 255.0f };

} // namespace math
} // namespace rt

#include "Vector8Impl.h"