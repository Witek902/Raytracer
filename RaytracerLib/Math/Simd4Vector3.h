#pragma once

#include "Math.h"
#include "Vector4.h"


namespace rt {
namespace math {

/**
 * 4 3D vectors (SIMD version).
 */
class RT_ALIGN(16) Vector3_Simd4
{
public:
    Vector4 x;
    Vector4 y;
    Vector4 z;

    Vector3_Simd4() = default;
    Vector3_Simd4(const Vector3_Simd4&) = default;
    Vector3_Simd4& operator = (const Vector3_Simd4&) = default;

    RT_FORCE_INLINE Vector3_Simd4(const Vector4& x, const Vector4& y, const Vector4& z)
        : x(x), y(y), z(z)
    { }

    // splat a single 3D vector
    RT_FORCE_INLINE Vector3_Simd4(const Vector4& v)
    {
        x = v.SplatX();
        y = v.SplatY();
        z = v.SplatZ();
    }

    // build from four 3D vectors
    RT_FORCE_INLINE Vector3_Simd4(const Vector4& v0, const Vector4& v1, const Vector4& v2, const Vector4& v3)
    {
        // transpose
        const __m128 tmp0 = _mm_shuffle_ps(v0, v1, 0x44);
        const __m128 tmp1 = _mm_shuffle_ps(v2, v3, 0x44);
        const __m128 tmp2 = _mm_shuffle_ps(v0, v1, 0xEE);
        const __m128 tmp3 = _mm_shuffle_ps(v2, v3, 0xEE);
        x = _mm_shuffle_ps(tmp0, tmp1, 0x88);
        y = _mm_shuffle_ps(tmp0, tmp1, 0xDD);
        z = _mm_shuffle_ps(tmp2, tmp3, 0x88);
    }

    //////////////////////////////////////////////////////////////////////////

    RT_FORCE_INLINE Vector3_Simd4 operator + (const Vector3_Simd4& rhs) const
    {
        return Vector3_Simd4(
            x + rhs.x,
            y + rhs.y,
            z + rhs.z
        );
    }

    RT_FORCE_INLINE Vector3_Simd4 operator - (const Vector3_Simd4& rhs) const
    {
        return Vector3_Simd4(
            x - rhs.x,
            y - rhs.y,
            z - rhs.z
        );
    }

    RT_FORCE_INLINE Vector3_Simd4 operator * (const Vector3_Simd4& rhs) const
    {
        return Vector3_Simd4(
            x * rhs.x,
            y * rhs.y,
            z * rhs.z
        );
    }

    RT_FORCE_INLINE Vector3_Simd4 operator * (const float rhs) const
    {
        return Vector3_Simd4(x * rhs, y * rhs, z * rhs);
    }

    RT_FORCE_INLINE static Vector3_Simd4 FastReciprocal(const Vector3_Simd4& v)
    {
        return Vector3_Simd4(
            Vector4::FastReciprocal(v.x),
            Vector4::FastReciprocal(v.y),
            Vector4::FastReciprocal(v.z)
        );
    }

    //////////////////////////////////////////////////////////////////////////

    // 3D dot product
    RT_FORCE_INLINE static Vector4 Dot(const Vector3_Simd4& a, const Vector3_Simd4& b)
    {
        return Vector4::MulAndAdd(a.x, b.x, Vector4::MulAndAdd(a.y, b.y, a.z * b.z));
    }

    // 3D cross product
    RT_FORCE_INLINE static Vector3_Simd4 Cross(const Vector3_Simd4& a, const Vector3_Simd4& b)
    {
        return Vector3_Simd4(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );

        /*
        return Vector3_Simd4(
            Vector4::NegMulAndAdd(a.z, b.y, a.y * b.z),
            Vector4::NegMulAndAdd(a.x, b.z, a.z * b.x),
            Vector4::NegMulAndAdd(a.y, b.x, a.x * b.y)
        );
        */
    }

    //////////////////////////////////////////////////////////////////////////

    RT_FORCE_INLINE static Vector3_Simd4 Min(const Vector3_Simd4& a, const Vector3_Simd4& b)
    {
        return Vector3_Simd4(
            Vector4::Min(a.x, b.x),
            Vector4::Min(a.y, b.y),
            Vector4::Min(a.z, b.z)
        );
    }

    RT_FORCE_INLINE static Vector3_Simd4 Max(const Vector3_Simd4& a, const Vector3_Simd4& b)
    {
        return Vector3_Simd4(
            Vector4::Max(a.x, b.x),
            Vector4::Max(a.y, b.y),
            Vector4::Max(a.z, b.z)
        );
    }
};


} // namespace math
} // namespace rt
