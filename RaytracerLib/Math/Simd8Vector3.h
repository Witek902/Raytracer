#pragma once

#include "Math.h"
#include "Vector8.h"


namespace rt {
namespace math {

/**
 * Eight 3D vectors (SIMD version, AVX accelerated).
 */
class RT_ALIGN(32) Vector3_Simd8
{
public:
    Vector8 x;
    Vector8 y;
    Vector8 z;

    Vector3_Simd8() = default;
    Vector3_Simd8(const Vector3_Simd8&) = default;
    Vector3_Simd8& operator = (const Vector3_Simd8&) = default;

    RT_FORCE_INLINE Vector3_Simd8(const Vector8& x, const Vector8& y, const Vector8& z)
        : x(x), y(y), z(z)
    { }

    // splat single 3D vector
    RT_FORCE_INLINE Vector3_Simd8(const Vector4& v)
    {
        const Vector8 temp(v, v); // copy "v" onto both AVX lanes
        x = _mm256_shuffle_ps(temp, temp, _MM_SHUFFLE(0, 0, 0, 0));
        y = _mm256_shuffle_ps(temp, temp, _MM_SHUFFLE(1, 1, 1, 1));
        z = _mm256_shuffle_ps(temp, temp, _MM_SHUFFLE(2, 2, 2, 2));
    }

    // build from eight 3D vectors
    RT_FORCE_INLINE Vector3_Simd8(const Vector4& v0, const Vector4& v1, const Vector4& v2, const Vector4& v3,
                                  const Vector4& v4, const Vector4& v5, const Vector4& v6, const Vector4& v7)
    {
        // TODO this can be probably optimized somehow

        // What is going on here is basically converting this:
        //
        // v0 = [ v0.x, v0.y, v0.z, v0.w ]
        // v1 = [ v1.x, v1.y, v1.z, v1.w ]
        // v2 = [ v2.x, v2.y, v2.z, v2.w ]
        // v3 = [ v3.x, v3.y, v3.z, v3.w ]
        // v4 = [ v4.x, v4.y, v4.z, v4.w ]
        // v5 = [ v5.x, v5.y, v5.z, v5.w ]
        // v6 = [ v6.x, v6.y, v6.z, v6.w ]
        // v7 = [ v7.x, v7.y, v7.z, v7.w ]
        //
        // into this:
        //
        // x = [ v0.x, v1.x, v2.x, v3.x, v4.x, v5.x, v6.x, v7.x ]
        // y = [ v0.y, v1.y, v2.y, v3.y, v4.y, v5.y, v6.y, v7.y ]
        // z = [ v0.z, v1.z, v2.z, v3.z, v4.z, v5.z, v6.z, v7.z ]
        //
        // note that "w" component is dropped

        const __m256 t0 = _mm256_unpacklo_ps(Vector8(v0), Vector8(v1));
        const __m256 t1 = _mm256_unpackhi_ps(Vector8(v0), Vector8(v1));
        const __m256 t2 = _mm256_unpacklo_ps(Vector8(v2), Vector8(v3));
        const __m256 t3 = _mm256_unpackhi_ps(Vector8(v2), Vector8(v3));
        const __m256 t4 = _mm256_unpacklo_ps(Vector8(v4), Vector8(v5));
        const __m256 t5 = _mm256_unpackhi_ps(Vector8(v4), Vector8(v5));
        const __m256 t6 = _mm256_unpacklo_ps(Vector8(v6), Vector8(v7));
        const __m256 t7 = _mm256_unpackhi_ps(Vector8(v6), Vector8(v7));
        const __m256 tt0 = _mm256_shuffle_ps(t0, t2, _MM_SHUFFLE(1, 0, 1, 0));
        const __m256 tt1 = _mm256_shuffle_ps(t0, t2, _MM_SHUFFLE(3, 2, 3, 2));
        const __m256 tt2 = _mm256_shuffle_ps(t1, t3, _MM_SHUFFLE(1, 0, 1, 0));
        const __m256 tt4 = _mm256_shuffle_ps(t4, t6, _MM_SHUFFLE(1, 0, 1, 0));
        const __m256 tt5 = _mm256_shuffle_ps(t4, t6, _MM_SHUFFLE(3, 2, 3, 2));
        const __m256 tt6 = _mm256_shuffle_ps(t5, t7, _MM_SHUFFLE(1, 0, 1, 0));
        x = _mm256_permute2f128_ps(tt0, tt4, 0x20);
        y = _mm256_permute2f128_ps(tt1, tt5, 0x20);
        z = _mm256_permute2f128_ps(tt2, tt6, 0x20);
    }

    //////////////////////////////////////////////////////////////////////////

    RT_FORCE_INLINE Vector3_Simd8 operator + (const Vector3_Simd8& rhs) const
    {
        return Vector3_Simd8(x + rhs.x, y + rhs.y, z + rhs.z);
    }

    RT_FORCE_INLINE Vector3_Simd8 operator - (const Vector3_Simd8& rhs) const
    {
        return Vector3_Simd8(x - rhs.x, y - rhs.y, z - rhs.z);
    }

    RT_FORCE_INLINE Vector3_Simd8 operator * (const Vector3_Simd8& rhs) const
    {
        return Vector3_Simd8(x * rhs.x, y * rhs.y,z * rhs.z);
    }

    RT_FORCE_INLINE Vector3_Simd8 operator / (const Vector3_Simd8& rhs) const
    {
        return Vector3_Simd8(x / rhs.x, y / rhs.y, z / rhs.z);
    }

    RT_FORCE_INLINE Vector3_Simd8 operator * (const float rhs) const
    {
        return Vector3_Simd8(x * rhs, y * rhs, z * rhs);
    }

    //////////////////////////////////////////////////////////////////////////

    RT_FORCE_INLINE Vector3_Simd8& operator += (const Vector3_Simd8& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

    RT_FORCE_INLINE Vector3_Simd8& operator -= (const Vector3_Simd8& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }

    RT_FORCE_INLINE Vector3_Simd8& operator *= (const Vector3_Simd8& rhs)
    {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        return *this;
    }

    RT_FORCE_INLINE Vector3_Simd8& operator /= (const Vector3_Simd8& rhs)
    {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;
        return *this;
    }

    RT_FORCE_INLINE Vector3_Simd8& operator *= (const float rhs)
    {
        x *= rhs;
        y *= rhs;
        z *= rhs;
        return *this;
    }

    RT_FORCE_INLINE static Vector3_Simd8 FastReciprocal(const Vector3_Simd8& v)
    {
        return Vector3_Simd8(
            Vector8::FastReciprocal(v.x),
            Vector8::FastReciprocal(v.y),
            Vector8::FastReciprocal(v.z)
        );
    }

    //////////////////////////////////////////////////////////////////////////

    // 3D dot product
    RT_FORCE_INLINE static Vector8 Dot(const Vector3_Simd8& a, const Vector3_Simd8& b)
    {
        // return a.x * b.x + a.y * b.y + a.z * b.z;
        return Vector8::MulAndAdd(a.x, b.x, Vector8::MulAndAdd(a.y, b.y, a.z * b.z));
    }

    // 3D cross product
    RT_FORCE_INLINE static Vector3_Simd8 Cross(const Vector3_Simd8& a, const Vector3_Simd8& b)
    {
        return Vector3_Simd8(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );

        /*
        return Vector3_Simd8(
            Vector8::NegMulAndAdd(a.z, b.y, a.y * b.z),
            Vector8::NegMulAndAdd(a.x, b.z, a.z * b.x),
            Vector8::NegMulAndAdd(a.y, b.x, a.x * b.y) 
        );
        */
    }

    //////////////////////////////////////////////////////////////////////////

    RT_FORCE_INLINE static Vector3_Simd8 Min(const Vector3_Simd8& a, const Vector3_Simd8& b)
    {
        return Vector3_Simd8(Vector8::Min(a.x, b.x), Vector8::Min(a.y, b.y), Vector8::Min(a.z, b.z));
    }

    RT_FORCE_INLINE static Vector3_Simd8 Max(const Vector3_Simd8& a, const Vector3_Simd8& b)
    {
        return Vector3_Simd8(Vector8::Max(a.x, b.x), Vector8::Max(a.y, b.y), Vector8::Max(a.z, b.z));
    }

};


} // namespace math
} // namespace rt
