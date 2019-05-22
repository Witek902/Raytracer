#pragma once

#include "Math.h"
#include "VectorBool4.h"

namespace rt {
namespace math {

struct Vector4;

/**
 * 4-element signed integer SIMD vector.
 */
struct RT_ALIGN(16) VectorInt4
{
    union
    {
        __m128i v;
        Int32 i[4];

        struct
        {
            Int32 x;
            Int32 y;
            Int32 z;
            Int32 w;
        };
    };

    // constructors
    RT_FORCE_INLINE VectorInt4() = default;
    RT_FORCE_INLINE static const VectorInt4 Zero();
    RT_FORCE_INLINE VectorInt4(const __m128i& m);
    RT_FORCE_INLINE VectorInt4(const VectorInt4& other);
    RT_FORCE_INLINE explicit VectorInt4(const Int32 scalar);
    RT_FORCE_INLINE explicit VectorInt4(const Uint32 scalar);
    RT_FORCE_INLINE VectorInt4(const Int32 x, const Int32 y, const Int32 z, const Int32 w);
    RT_FORCE_INLINE operator __m128i() const { return v; }

    RT_FORCE_INLINE Int32 operator[] (const Uint32 index) const { return i[index]; }

    // bitwise logic operations
    RT_FORCE_INLINE const VectorInt4 operator & (const VectorInt4& b) const;
    RT_FORCE_INLINE const VectorInt4 operator | (const VectorInt4& b) const;
    RT_FORCE_INLINE const VectorInt4 operator ^ (const VectorInt4& b) const;
    RT_FORCE_INLINE VectorInt4& operator &= (const VectorInt4& b);
    RT_FORCE_INLINE VectorInt4& operator |= (const VectorInt4& b);
    RT_FORCE_INLINE VectorInt4& operator ^= (const VectorInt4& b);

    // simple arithmetics
    RT_FORCE_INLINE const VectorInt4 operator - () const;
    RT_FORCE_INLINE const VectorInt4 operator + (const VectorInt4& b) const;
    RT_FORCE_INLINE const VectorInt4 operator - (const VectorInt4& b) const;
    RT_FORCE_INLINE const VectorInt4 operator * (const VectorInt4& b) const;
    RT_FORCE_INLINE VectorInt4& operator += (const VectorInt4& b);
    RT_FORCE_INLINE VectorInt4& operator -= (const VectorInt4& b);
    RT_FORCE_INLINE VectorInt4& operator *= (const VectorInt4& b);
    RT_FORCE_INLINE const VectorInt4 operator + (Int32 b) const;
    RT_FORCE_INLINE const VectorInt4 operator - (Int32 b) const;
    RT_FORCE_INLINE const VectorInt4 operator * (Int32 b) const;
    RT_FORCE_INLINE VectorInt4& operator += (Int32 b);
    RT_FORCE_INLINE VectorInt4& operator -= (Int32 b);
    RT_FORCE_INLINE VectorInt4& operator *= (Int32 b);

    // bit shifting
    RT_FORCE_INLINE const VectorInt4 operator << (Int32 b) const;
    RT_FORCE_INLINE const VectorInt4 operator >> (Int32 b) const;
    RT_FORCE_INLINE VectorInt4& operator <<= (Int32 b);
    RT_FORCE_INLINE VectorInt4& operator >>= (Int32 b);

    // for each component, if it's greater-or-euqal to 'reference', set the value to 'target'
    RT_FORCE_INLINE const VectorInt4 SetIfGreaterOrEqual(const VectorInt4& reference, const VectorInt4& target) const;
    // for each component, if it's less than 'reference', set the value to 'target'
    RT_FORCE_INLINE const VectorInt4 SetIfLessThan(const VectorInt4& reference, const VectorInt4& target) const;

    RT_FORCE_INLINE const VectorBool4 operator == (const VectorInt4& b) const;
    RT_FORCE_INLINE const VectorBool4 operator < (const VectorInt4& b) const;
    RT_FORCE_INLINE const VectorBool4 operator <= (const VectorInt4& b) const;
    RT_FORCE_INLINE const VectorBool4 operator > (const VectorInt4& b) const;
    RT_FORCE_INLINE const VectorBool4 operator >= (const VectorInt4& b) const;
    RT_FORCE_INLINE const VectorBool4 operator != (const VectorInt4& b) const;

    RT_FORCE_INLINE static const VectorInt4 Min(const VectorInt4& a, const VectorInt4& b);
    RT_FORCE_INLINE static const VectorInt4 Max(const VectorInt4& a, const VectorInt4& b);

    RT_FORCE_INLINE const VectorInt4 Clamped(const VectorInt4& min, const VectorInt4& max) const;

    // convert from float vector to integer vector
    RT_FORCE_INLINE static const VectorInt4 Convert(const Vector4& v);

    // convert to float vector
    RT_FORCE_INLINE const Vector4 ConvertToFloat() const;

    // cast from float vector (preserve bits)
    RT_FORCE_INLINE static const VectorInt4 Cast(const Vector4& v);

    // convert to float vector
    RT_FORCE_INLINE const Vector4 CastToFloat() const;

    // Rearrange vector elements
    template<Uint32 ix, Uint32 iy, Uint32 iz, Uint32 iw>
    RT_FORCE_INLINE const VectorInt4 Swizzle() const;
};

} // namespace math
} // namespace rt


#include "VectorInt4Impl.h"