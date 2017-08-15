#pragma once

#include "Math.h"
#include "Vector4.h"

#define RT_USE_FMA

namespace rt {
namespace math {

/**
 * 8-element SIMD vector.
 */
struct RT_ALIGN(32) Vector8
{
    union
    {
        Float f[8];
        int i[8];
        unsigned int u[8];
        __m256 v;
    };

    // conversion to/from AVX types
    RT_FORCE_INLINE operator __m256() const { return v; }
    RT_FORCE_INLINE operator __m256() { return v; }
    RT_FORCE_INLINE operator __m256i() const { return reinterpret_cast<const __m256i*>(&v)[0]; }

    // constructors
    RT_FORCE_INLINE Vector8();
    RT_FORCE_INLINE Vector8(const __m256& m);
    RT_FORCE_INLINE explicit Vector8(const Vector4& lo);
    RT_FORCE_INLINE Vector8(const Vector4& lo, const Vector4& hi);
    RT_FORCE_INLINE Vector8(Float e0, Float e1 = 0.0f, Float e2 = 0.0f, Float e3 = 0.0f, Float e4 = 0.0f, Float e5 = 0.0f, Float e6 = 0.0f, Float e7 = 0.0f);
    RT_FORCE_INLINE Vector8(int e0, int e1 = 0, int e2 = 0, int e3 = 0, int e4 = 0, int e5 = 0, int e6 = 0, int e7 = 0);
    RT_FORCE_INLINE Vector8(const Float* src);
    RT_FORCE_INLINE void Set(Float scalar);

    // element access
    RT_FORCE_INLINE Float operator[] (int index) const
    {
        return f[index];
    }

    // element access (reference)
    RT_FORCE_INLINE Float& operator[] (int index)
    {
        return f[index];
    }

    /// simple arithmetics
    RT_FORCE_INLINE Vector8 operator- () const;
    RT_FORCE_INLINE Vector8 operator+ (const Vector8& b) const;
    RT_FORCE_INLINE Vector8 operator- (const Vector8& b) const;
    RT_FORCE_INLINE Vector8 operator* (const Vector8& b) const;
    RT_FORCE_INLINE Vector8 operator/ (const Vector8& b) const;
    RT_FORCE_INLINE Vector8 operator* (Float b) const;
    RT_FORCE_INLINE Vector8 operator/ (Float b) const;
    RT_FORCE_INLINE Vector8& operator+= (const Vector8& b);
    RT_FORCE_INLINE Vector8& operator-= (const Vector8& b);
    RT_FORCE_INLINE Vector8& operator*= (const Vector8& b);
    RT_FORCE_INLINE Vector8& operator/= (const Vector8& b);
    RT_FORCE_INLINE Vector8& operator*= (Float b);
    RT_FORCE_INLINE Vector8& operator/= (Float b);

    /// comparison operators (returns true, if all the elements satisfy the equation)
    RT_FORCE_INLINE bool operator== (const Vector8& b) const;
    RT_FORCE_INLINE bool operator< (const Vector8& b) const;
    RT_FORCE_INLINE bool operator<= (const Vector8& b) const;
    RT_FORCE_INLINE bool operator> (const Vector8& b) const;
    RT_FORCE_INLINE bool operator>= (const Vector8& b) const;
    RT_FORCE_INLINE bool operator!= (const Vector8& b) const;

    /// bitwise logic operations
    RT_FORCE_INLINE Vector8 operator& (const Vector8& b) const;
    RT_FORCE_INLINE Vector8 operator| (const Vector8& b) const;
    RT_FORCE_INLINE Vector8 operator^ (const Vector8& b) const;
    RT_FORCE_INLINE Vector8& operator&= (const Vector8& b);
    RT_FORCE_INLINE Vector8& operator|= (const Vector8& b);
    RT_FORCE_INLINE Vector8& operator^= (const Vector8& b);
    RT_FORCE_INLINE static Vector8 Splat(Float f);

    RT_FORCE_INLINE static Vector8 Floor(const Vector8& v);
    RT_FORCE_INLINE static Vector8 Sqrt(const Vector8& v);
    RT_FORCE_INLINE static Vector8 Reciprocal(const Vector8& v);
    RT_FORCE_INLINE static Vector8 FastReciprocal(const Vector8& v);
    RT_FORCE_INLINE static Vector8 Lerp(const Vector8& v1, const Vector8& v2, const Vector8& weight);
    RT_FORCE_INLINE static Vector8 Lerp(const Vector8& v1, const Vector8& v2, Float weight);
    RT_FORCE_INLINE static Vector8 Min(const Vector8& a, const Vector8& b);
    RT_FORCE_INLINE static Vector8 Max(const Vector8& a, const Vector8& b);
    RT_FORCE_INLINE static Vector8 Abs(const Vector8& v);
    RT_FORCE_INLINE Vector8 Clamped(const Vector8& min, const Vector8& max) const;

    RT_FORCE_INLINE static int EqualMask(const Vector8& v1, const Vector8& v2);
    RT_FORCE_INLINE static int LessMask(const Vector8& v1, const Vector8& v2);
    RT_FORCE_INLINE static int LessEqMask(const Vector8& v1, const Vector8& v2);
    RT_FORCE_INLINE static int GreaterMask(const Vector8& v1, const Vector8& v2);
    RT_FORCE_INLINE static int GreaterEqMask(const Vector8& v1, const Vector8& v2);
    RT_FORCE_INLINE static int NotEqualMask(const Vector8& v1, const Vector8& v2);

    /**
     * Build mask of sign bits.
     */
    RT_FORCE_INLINE int GetSignMask() const;

    
    /**
     * For each vector component, copy value from "a" if "sel" > 0.0f, or from "b" otherwise.
     */
    RT_FORCE_INLINE static Vector8 SelectBySign(const Vector8& a, const Vector8& b, const Vector8& sel);

    /**
     * Check if two vectors are (almost) equal.
     */
    RT_FORCE_INLINE static bool AlmostEqual(const Vector8& v1, const Vector8& v2, Float epsilon = RT_EPSILON)
    {
        const Vector8 diff = Abs(v1 - v2);
        const Vector8 epsilonV = Vector8::Splat(epsilon);
        return diff < epsilonV;
    }

    /**
     * Fused multiply and add.
     * @return  a * b + c
     */
    RT_FORCE_INLINE static Vector8 MulAndAdd(const Vector8& a, const Vector8& b, const Vector8& c);

    /**
     * Fused multiply and subtract.
     * @return  a * b - c
     */
    RT_FORCE_INLINE static Vector8 MulAndSub(const Vector8& a, const Vector8& b, const Vector8& c);
        
    /**
     * Fused multiply (negated) and add.
     * @return  - a * b + c
     */
    RT_FORCE_INLINE static Vector8 NegMulAndAdd(const Vector8& a, const Vector8& b, const Vector8& c);

    /**
     * Fused multiply (negated) and subtract.
     * @return  - a * b - c
     */
    RT_FORCE_INLINE static Vector8 NegMulAndSub(const Vector8& a, const Vector8& b, const Vector8& c);

    /**
     * Calculate horizontal minimum. Result is splatted across all elements.
     */
    RT_FORCE_INLINE Vector8 HorizontalMin() const;

    /**
     * Calculate horizontal maximum. Result is splatted across all elements.
     */
    RT_FORCE_INLINE Vector8 HorizontalMax() const;
};

// like Vector8::operator * (Float)
RT_FORCE_INLINE Vector8 operator*(Float a, const Vector8& b);


} // namespace math
} // namespace rt


#include "Vector8Constants.h"
#include "Vector8Impl.h"