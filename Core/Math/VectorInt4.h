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
        Int32 i[4];
        Int64 i64[2];


#ifdef RT_USE_SSE
        __m128i v;
#endif // RT_USE_SSE

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
    RT_FORCE_INLINE VectorInt4(const VectorInt4& other);
    RT_FORCE_INLINE VectorInt4(const VectorBool4& other);
    RT_FORCE_INLINE explicit VectorInt4(const Int32 scalar);
    RT_FORCE_INLINE explicit VectorInt4(const Uint32 scalar);
    RT_FORCE_INLINE VectorInt4(const Int32 x, const Int32 y, const Int32 z, const Int32 w);

#ifdef RT_USE_SSE
    RT_FORCE_INLINE VectorInt4(const __m128i& m);
    RT_FORCE_INLINE operator __m128i() const { return v; }
#endif // RT_USE_SSE

    RT_FORCE_INLINE Int32 operator[] (const Uint32 index) const { return i[index]; }

    // bitwise logic operations
    RT_FORCE_INLINE const VectorInt4 operator & (const VectorInt4& b) const;
    RT_FORCE_INLINE const VectorInt4 operator | (const VectorInt4& b) const;
    RT_FORCE_INLINE const VectorInt4 operator ^ (const VectorInt4& b) const;
    RT_FORCE_INLINE VectorInt4& operator &= (const VectorInt4& b);
    RT_FORCE_INLINE VectorInt4& operator |= (const VectorInt4& b);
    RT_FORCE_INLINE VectorInt4& operator ^= (const VectorInt4& b);
    RT_FORCE_INLINE static const VectorInt4 AndNot(const VectorInt4& a, const VectorInt4& b);

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
    RT_FORCE_INLINE const VectorInt4 operator << (const VectorInt4& b) const;
    RT_FORCE_INLINE const VectorInt4 operator >> (const VectorInt4& b) const;
    RT_FORCE_INLINE VectorInt4& operator <<= (const VectorInt4& b);
    RT_FORCE_INLINE VectorInt4& operator >>= (const VectorInt4& b);
    RT_FORCE_INLINE const VectorInt4 operator << (Int32 b) const;
    RT_FORCE_INLINE const VectorInt4 operator >> (Int32 b) const;
    RT_FORCE_INLINE VectorInt4& operator <<= (Int32 b);
    RT_FORCE_INLINE VectorInt4& operator >>= (Int32 b);

    // For each vector component, copy value from "a" if "sel" is "false", or from "b" otherwise
    RT_FORCE_INLINE static const VectorInt4 Select(const VectorInt4& a, const VectorInt4& b, const VectorBool4& sel);

    RT_FORCE_INLINE const VectorBool4 operator == (const VectorInt4& b) const;
    RT_FORCE_INLINE const VectorBool4 operator < (const VectorInt4& b) const;
    RT_FORCE_INLINE const VectorBool4 operator <= (const VectorInt4& b) const;
    RT_FORCE_INLINE const VectorBool4 operator > (const VectorInt4& b) const;
    RT_FORCE_INLINE const VectorBool4 operator >= (const VectorInt4& b) const;
    RT_FORCE_INLINE const VectorBool4 operator != (const VectorInt4& b) const;

    RT_FORCE_INLINE static const VectorInt4 Min(const VectorInt4& a, const VectorInt4& b);
    RT_FORCE_INLINE static const VectorInt4 Max(const VectorInt4& a, const VectorInt4& b);

    RT_FORCE_INLINE const VectorInt4 Clamped(const VectorInt4& min, const VectorInt4& max) const;

    // convert from float vector to integer vector (with rounding)
    RT_FORCE_INLINE static const VectorInt4 Convert(const Vector4& v);

    // convert from float vector to integer vector (with truncation towards zero)
    RT_FORCE_INLINE static const VectorInt4 TruncateAndConvert(const Vector4& v);

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


#ifdef RT_USE_SSE
#include "VectorInt4ImplSSE.h"
#else
#include "VectorInt4ImplNaive.h"
#endif
