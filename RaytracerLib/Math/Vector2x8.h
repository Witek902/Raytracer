#pragma once

#include "Math.h"
#include "Vector8.h"


namespace rt {
namespace math {

/**
 * Eight 2D vectors (SIMD version, AVX accelerated).
 */
class RT_ALIGN(32) Vector2x8
{
public:
    Vector8 x;
    Vector8 y;

    Vector2x8() = default;
    Vector2x8(const Vector2x8&) = default;
    Vector2x8& operator = (const Vector2x8&) = default;

    RT_FORCE_INLINE Vector2x8(const Vector8& x, const Vector8& y)
        : x(x), y(y)
    { }

    // splat single 3D vector
    RT_FORCE_INLINE explicit Vector2x8(const Vector4& v)
    {
        const Vector8 temp(v, v); // copy "v" onto both AVX lanes
        x = _mm256_shuffle_ps(temp, temp, _MM_SHUFFLE(0, 0, 0, 0));
        y = _mm256_shuffle_ps(temp, temp, _MM_SHUFFLE(1, 1, 1, 1));
    }

    // build from eight 3D vectors
    RT_FORCE_INLINE Vector2x8(const Vector4& v0, const Vector4& v1, const Vector4& v2, const Vector4& v3,
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
        //
        // note that "z" and "w" component are dropped

        const __m256 t0 = _mm256_unpacklo_ps(Vector8(v0), Vector8(v1));
        const __m256 t2 = _mm256_unpacklo_ps(Vector8(v2), Vector8(v3));
        const __m256 t4 = _mm256_unpacklo_ps(Vector8(v4), Vector8(v5));
        const __m256 t6 = _mm256_unpacklo_ps(Vector8(v6), Vector8(v7));
        const __m256 tt0 = _mm256_shuffle_ps(t0, t2, _MM_SHUFFLE(1, 0, 1, 0));
        const __m256 tt1 = _mm256_shuffle_ps(t0, t2, _MM_SHUFFLE(3, 2, 3, 2));
        const __m256 tt4 = _mm256_shuffle_ps(t4, t6, _MM_SHUFFLE(1, 0, 1, 0));
        const __m256 tt5 = _mm256_shuffle_ps(t4, t6, _MM_SHUFFLE(3, 2, 3, 2));
        x = _mm256_permute2f128_ps(tt0, tt4, 0x20);
        y = _mm256_permute2f128_ps(tt1, tt5, 0x20);
    }

    RT_FORCE_INLINE static Vector2x8 One()
    {
        return
        {
            Vector8(1.0f),
            Vector8(1.0f),
        };
    }

    //////////////////////////////////////////////////////////////////////////

    RT_FORCE_INLINE Vector2x8 operator + (const Vector2x8& rhs) const
    {
        return { x + rhs.x, y + rhs.y };
    }

    RT_FORCE_INLINE Vector2x8 operator - (const Vector2x8& rhs) const
    {
        return { x - rhs.x, y - rhs.y };
    }

    RT_FORCE_INLINE Vector2x8 operator * (const Vector2x8& rhs) const
    {
        return { x * rhs.x, y * rhs.y };
    }

    RT_FORCE_INLINE Vector2x8 operator * (const Vector8& rhs) const
    {
        return { x * rhs, y * rhs };
    }

    RT_FORCE_INLINE Vector2x8 operator / (const Vector2x8& rhs) const
    {
        return { x / rhs.x, y / rhs.y };
    }

    RT_FORCE_INLINE Vector2x8 operator * (const float rhs) const
    {
        return { x * rhs, y * rhs };
    }

    //////////////////////////////////////////////////////////////////////////

    RT_FORCE_INLINE Vector2x8& operator += (const Vector2x8& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    RT_FORCE_INLINE Vector2x8& operator -= (const Vector2x8& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    RT_FORCE_INLINE Vector2x8& operator *= (const Vector2x8& rhs)
    {
        x *= rhs.x;
        y *= rhs.y;
        return *this;
    }

    RT_FORCE_INLINE Vector2x8& operator /= (const Vector2x8& rhs)
    {
        x /= rhs.x;
        y /= rhs.y;
        return *this;
    }

    RT_FORCE_INLINE Vector2x8& operator *= (const float rhs)
    {
        x *= rhs;
        y *= rhs;
        return *this;
    }

    RT_FORCE_INLINE static Vector2x8 FastReciprocal(const Vector2x8& v)
    {
        return
        {
            Vector8::FastReciprocal(v.x),
            Vector8::FastReciprocal(v.y)
        };
    }

    RT_FORCE_INLINE static Vector2x8 MulAndAdd(const Vector2x8& a, const Vector2x8& b, const Vector2x8& c)
    {
        return
        {
            Vector8::MulAndAdd(a.x, b.x, c.x),
            Vector8::MulAndAdd(a.y, b.y, c.y)
        };
    }

    RT_FORCE_INLINE static Vector2x8 MulAndSub(const Vector2x8& a, const Vector2x8& b, const Vector2x8& c)
    {
        return
        {
            Vector8::MulAndSub(a.x, b.x, c.x),
            Vector8::MulAndSub(a.y, b.y, c.y)
        };
    }

    RT_FORCE_INLINE static Vector2x8 NegMulAndAdd(const Vector2x8& a, const Vector2x8& b, const Vector2x8& c)
    {
        return
        {
            Vector8::NegMulAndAdd(a.x, b.x, c.x),
            Vector8::NegMulAndAdd(a.y, b.y, c.y)
        };
    }

    RT_FORCE_INLINE static Vector2x8 NegMulAndSub(const Vector2x8& a, const Vector2x8& b, const Vector2x8& c)
    {
        return
        {
            Vector8::NegMulAndSub(a.x, b.x, c.x),
            Vector8::NegMulAndSub(a.y, b.y, c.y)
        };
    }

    //////////////////////////////////////////////////////////////////////////

    // dot product
    RT_FORCE_INLINE static Vector8 Dot(const Vector2x8& a, const Vector2x8& b)
    {
        // return a.x * b.x + a.y * b.y
        return Vector8::MulAndAdd(a.x, b.x, a.y * b.y);
    }
    //////////////////////////////////////////////////////////////////////////

    RT_FORCE_INLINE static Vector2x8 Min(const Vector2x8& a, const Vector2x8& b)
    {
        return { Vector8::Min(a.x, b.x), Vector8::Min(a.y, b.y) };
    }

    RT_FORCE_INLINE static Vector2x8 Max(const Vector2x8& a, const Vector2x8& b)
    {
        return { Vector8::Max(a.x, b.x), Vector8::Max(a.y, b.y) };
    }

};


} // namespace math
} // namespace rt
