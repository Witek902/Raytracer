#pragma once

#include "Math.h"
#include "VectorInt4.h"

namespace rt {
namespace math {

struct Vector8;

/**
 * 8-element integer SIMD vector.
 */
struct RT_ALIGN(32) VectorInt8
{
    // constructors
    RT_FORCE_INLINE VectorInt8() = default;
    RT_FORCE_INLINE static const VectorInt8 Zero();
    RT_FORCE_INLINE VectorInt8(const __m256i& m);
    RT_FORCE_INLINE VectorInt8(const VectorInt4& lo, const VectorInt4& hi);
    RT_FORCE_INLINE explicit VectorInt8(const __m256& m);
    RT_FORCE_INLINE explicit VectorInt8(const Int32 scalar);
    RT_FORCE_INLINE explicit VectorInt8(const Uint32 scalar);
    RT_FORCE_INLINE VectorInt8(const Int32 e0, const Int32 e1, const Int32 e2, const Int32 e3, const Int32 e4, const Int32 e5, const Int32 e6, const Int32 e7);

    RT_FORCE_INLINE operator __m256i() const { return v; }
    RT_FORCE_INLINE operator __m256() const { return _mm256_castsi256_ps(v); }
    RT_FORCE_INLINE Int32 operator[] (const Uint32 index) const { return i[index]; }

    RT_FORCE_INLINE void SetElement(Uint32 index, Int32 value) { i[index] = value; }

    // bitwise logic operations
    RT_FORCE_INLINE const VectorInt8 operator & (const VectorInt8& b) const;
    RT_FORCE_INLINE const VectorInt8 operator | (const VectorInt8& b) const;
    RT_FORCE_INLINE const VectorInt8 operator ^ (const VectorInt8& b) const;
    RT_FORCE_INLINE VectorInt8& operator &= (const VectorInt8& b);
    RT_FORCE_INLINE VectorInt8& operator |= (const VectorInt8& b);
    RT_FORCE_INLINE VectorInt8& operator ^= (const VectorInt8& b);

#ifdef RT_USE_AVX2

    // simple arithmetics
    RT_FORCE_INLINE const VectorInt8 operator - () const;
    RT_FORCE_INLINE const VectorInt8 operator + (const VectorInt8& b) const;
    RT_FORCE_INLINE const VectorInt8 operator - (const VectorInt8& b) const;
    RT_FORCE_INLINE VectorInt8& operator += (const VectorInt8& b);
    RT_FORCE_INLINE VectorInt8& operator -= (const VectorInt8& b);
    RT_FORCE_INLINE const VectorInt8 operator + (Int32 b) const;
    RT_FORCE_INLINE const VectorInt8 operator - (Int32 b) const;
    RT_FORCE_INLINE VectorInt8& operator += (Int32 b);
    RT_FORCE_INLINE VectorInt8& operator -= (Int32 b);

    // bit shifting
    RT_FORCE_INLINE const VectorInt8 operator << (Int32 b) const;
    RT_FORCE_INLINE const VectorInt8 operator >> (Int32 b) const;

    /// comparison operators (returns true, if all the elements satisfy the equation)
    RT_FORCE_INLINE bool operator == (const VectorInt8& b) const;
    RT_FORCE_INLINE bool operator < (const VectorInt8& b) const;
    RT_FORCE_INLINE bool operator <= (const VectorInt8& b) const;
    RT_FORCE_INLINE bool operator > (const VectorInt8& b) const;
    RT_FORCE_INLINE bool operator >= (const VectorInt8& b) const;
    RT_FORCE_INLINE bool operator != (const VectorInt8& b) const;

    RT_FORCE_INLINE static const VectorInt8 Min(const VectorInt8& a, const VectorInt8& b);
    RT_FORCE_INLINE static const VectorInt8 Max(const VectorInt8& a, const VectorInt8& b);
    RT_FORCE_INLINE const VectorInt8 Clamped(const VectorInt8& min, const VectorInt8& max) const;

    // convert from float vector to integer vector
    RT_FORCE_INLINE static const VectorInt8 Convert(const Vector8& v);

    // convert to float vector
    RT_FORCE_INLINE const Vector8 ConvertToFloat() const;

#endif // RT_USE_AVX2

    // cast from float vector (preserve bits)
    RT_FORCE_INLINE static const VectorInt8 Cast(const Vector8& v);

    // convert to float vector
    RT_FORCE_INLINE const Vector8 CastToFloat() const;

    /**
     * Build mask of sign bits.
     */
    RT_FORCE_INLINE Int32 GetSignMask() const;

    /**
     * For each vector component, copy value from "a" if "sel" > 0.0f, or from "b" otherwise.
     */
    RT_FORCE_INLINE static const VectorInt8 SelectBySign(const VectorInt8& a, const VectorInt8& b, const VectorInt8& sel);

private:
    union
    {
        Int32 i[8];
        Uint32 u[8];
        __m256 f;
        __m256i v;
    };
};

} // namespace math
} // namespace rt


#include "VectorInt8Impl.h"