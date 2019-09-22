#pragma once

#include "Math.h"
#include "Vector8.h"

namespace rt {
namespace math {

/**
 * Eight 3D vectors (SIMD version, AVX accelerated).
 */
class RT_ALIGN(32) Vector3x8
{
public:
    Vector8 x;
    Vector8 y;
    Vector8 z;

    RT_FORCE_INLINE Vector3x8() = default;
    RT_FORCE_INLINE Vector3x8(const Vector3x8&) = default;
    RT_FORCE_INLINE Vector3x8& operator = (const Vector3x8&) = default;

    RT_FORCE_INLINE static const Vector3x8 Zero()
    {
        return { Vector8::Zero(), Vector8::Zero(), Vector8::Zero() };
    }

    RT_FORCE_INLINE static const Vector3x8 One()
    {
        return { Vector8(1.0f), Vector8(1.0f), Vector8(1.0f) };
    }

    RT_FORCE_INLINE Vector3x8(const Vector8& x, const Vector8& y, const Vector8& z)
        : x(x), y(y), z(z)
    {}

    // splat value to all the components
    RT_FORCE_INLINE explicit Vector3x8(const Vector8& s)
        : x(s), y(s), z(s)
    {}

    // splat single 3D vector
    RT_FORCE_INLINE explicit Vector3x8(const Vector4& v)
    {
#ifdef RT_USE_AVX
        const Vector8 temp{ v, v }; // copy "v" onto both AVX lanes
        x = _mm256_shuffle_ps(temp, temp, _MM_SHUFFLE(0, 0, 0, 0));
        y = _mm256_shuffle_ps(temp, temp, _MM_SHUFFLE(1, 1, 1, 1));
        z = _mm256_shuffle_ps(temp, temp, _MM_SHUFFLE(2, 2, 2, 2));
#else
        x = Vector8{ v.x };
        y = Vector8{ v.y };
        z = Vector8{ v.z };
#endif // RT_USE_AVX
    }

    // splat single scalar to all components an elements
    RT_FORCE_INLINE explicit Vector3x8(const float f)
        : x(f) , y(f) , z(f)
    {}

    // splat single 3D vector
    RT_FORCE_INLINE explicit Vector3x8(const Float3& v)
        : x(v.x), y(v.y), z(v.z)
    {}

    // build from eight 3D vectors
    RT_FORCE_INLINE Vector3x8(const Vector4& v0, const Vector4& v1, const Vector4& v2, const Vector4& v3,
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

#ifdef RT_USE_AVX

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

#else // !RT_USE_AVX

        x = Vector8{ v0.x, v1.x, v2.x, v3.x, v4.x, v5.x, v6.x, v7.x };
        y = Vector8{ v0.y, v1.y, v2.y, v3.y, v4.y, v5.y, v6.y, v7.y };
        z = Vector8{ v0.z, v1.z, v2.z, v3.z, v4.z, v5.z, v6.z, v7.z };

#endif // RT_USE_AVX
    }

    // unpack to 8x Vector4
    RT_FORCE_INLINE void Unpack(Vector4 output[8]) const
    {
#ifdef RT_USE_AVX

        __m256 row0 = x;
        __m256 row1 = y;
        __m256 row2 = z;
        __m256 row3 = _mm256_setzero_ps();
        __m256 tmp3, tmp2, tmp1, tmp0;

        tmp0 = _mm256_shuffle_ps(row0, row1, 0x44);
        tmp2 = _mm256_shuffle_ps(row0, row1, 0xEE);
        tmp1 = _mm256_shuffle_ps(row2, row3, 0x44);
        tmp3 = _mm256_shuffle_ps(row2, row3, 0xEE);
        row0 = _mm256_shuffle_ps(tmp0, tmp1, 0x88);
        row1 = _mm256_shuffle_ps(tmp0, tmp1, 0xDD);
        row2 = _mm256_shuffle_ps(tmp2, tmp3, 0x88);
        row3 = _mm256_shuffle_ps(tmp2, tmp3, 0xDD);

        output[0] = _mm256_castps256_ps128(row0);
        output[1] = _mm256_castps256_ps128(row1);
        output[2] = _mm256_castps256_ps128(row2);
        output[3] = _mm256_castps256_ps128(row3);
        output[4] = _mm256_extractf128_ps(row0, 1);
        output[5] = _mm256_extractf128_ps(row1, 1);
        output[6] = _mm256_extractf128_ps(row2, 1);
        output[7] = _mm256_extractf128_ps(row3, 1);

#else // !RT_USE_AVX

        for (uint32 i = 0; i < 8; ++i)
        {
            output[i] = Vector4(x[i], y[i], z[i]);
        }

#endif // RT_USE_AVX
    }

    //////////////////////////////////////////////////////////////////////////

    RT_FORCE_INLINE const Vector3x8 operator - () const
    {
        return { -x, -y, -z };
    }

    RT_FORCE_INLINE const Vector3x8 operator + (const Vector3x8& rhs) const
    {
        return { x + rhs.x, y + rhs.y, z + rhs.z };
    }

    RT_FORCE_INLINE const Vector3x8 operator - (const Vector3x8& rhs) const
    {
        return { x - rhs.x, y - rhs.y, z - rhs.z };
    }

    RT_FORCE_INLINE const Vector3x8 operator * (const Vector3x8& rhs) const
    {
        return { x * rhs.x, y * rhs.y,z * rhs.z };
    }

    RT_FORCE_INLINE const Vector3x8 operator * (const Vector8& rhs) const
    {
        return { x * rhs, y * rhs, z * rhs };
    }

    RT_FORCE_INLINE const Vector3x8 operator / (const Vector3x8& rhs) const
    {
        return { x / rhs.x, y / rhs.y, z / rhs.z };
    }

    RT_FORCE_INLINE const Vector3x8 operator * (const float rhs) const
    {
        return { x * rhs, y * rhs, z * rhs };
    }

    //////////////////////////////////////////////////////////////////////////

    RT_FORCE_INLINE Vector3x8& operator += (const Vector3x8& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

    RT_FORCE_INLINE Vector3x8& operator -= (const Vector3x8& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }

    RT_FORCE_INLINE Vector3x8& operator *= (const Vector3x8& rhs)
    {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        return *this;
    }

    RT_FORCE_INLINE Vector3x8& operator /= (const Vector3x8& rhs)
    {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;
        return *this;
    }

    RT_FORCE_INLINE Vector3x8& operator *= (const Vector8& rhs)
    {
        x *= rhs;
        y *= rhs;
        z *= rhs;
        return *this;
    }

    RT_FORCE_INLINE Vector3x8& operator *= (const float rhs)
    {
        x *= rhs;
        y *= rhs;
        z *= rhs;
        return *this;
    }

    RT_FORCE_INLINE static const Vector3x8 Reciprocal(const Vector3x8& v)
    {
        return
        {
            Vector8::Reciprocal(v.x),
            Vector8::Reciprocal(v.y),
            Vector8::Reciprocal(v.z)
        };
    }

    RT_FORCE_INLINE static const Vector3x8 FastReciprocal(const Vector3x8& v)
    {
        return
        {
            Vector8::FastReciprocal(v.x),
            Vector8::FastReciprocal(v.y),
            Vector8::FastReciprocal(v.z)
        };
    }

    //////////////////////////////////////////////////////////////////////////

    RT_FORCE_INLINE static const Vector3x8 MulAndAdd(const Vector3x8& a, const Vector3x8& b, const Vector3x8& c)
    {
        return { Vector8::MulAndAdd(a.x, b.x, c.x), Vector8::MulAndAdd(a.y, b.y, c.y), Vector8::MulAndAdd(a.z, b.z, c.z) };
    }

    RT_FORCE_INLINE static const Vector3x8 MulAndSub(const Vector3x8& a, const Vector3x8& b, const Vector3x8& c)
    {
        return { Vector8::MulAndSub(a.x, b.x, c.x), Vector8::MulAndSub(a.y, b.y, c.y), Vector8::MulAndSub(a.z, b.z, c.z) };
    }

    RT_FORCE_INLINE static const Vector3x8 NegMulAndAdd(const Vector3x8& a, const Vector3x8& b, const Vector3x8& c)
    {
        return { Vector8::NegMulAndAdd(a.x, b.x, c.x), Vector8::NegMulAndAdd(a.y, b.y, c.y), Vector8::NegMulAndAdd(a.z, b.z, c.z) };
    }

    RT_FORCE_INLINE static const Vector3x8 NegMulAndSub(const Vector3x8& a, const Vector3x8& b, const Vector3x8& c)
    {
        return { Vector8::NegMulAndSub(a.x, b.x, c.x), Vector8::NegMulAndSub(a.y, b.y, c.y), Vector8::NegMulAndSub(a.z, b.z, c.z) };
    }

    //////////////////////////////////////////////////////////////////////////

    RT_FORCE_INLINE static const Vector3x8 MulAndAdd(const Vector3x8& a, const Vector8& b, const Vector3x8& c)
    {
        return { Vector8::MulAndAdd(a.x, b, c.x), Vector8::MulAndAdd(a.y, b, c.y), Vector8::MulAndAdd(a.z, b, c.z) };
    }

    RT_FORCE_INLINE static const Vector3x8 MulAndSub(const Vector3x8& a, const Vector8& b, const Vector3x8& c)
    {
        return { Vector8::MulAndSub(a.x, b, c.x), Vector8::MulAndSub(a.y, b, c.y), Vector8::MulAndSub(a.z, b, c.z) };
    }

    RT_FORCE_INLINE static const Vector3x8 NegMulAndAdd(const Vector3x8& a, const Vector8& b, const Vector3x8& c)
    {
        return { Vector8::NegMulAndAdd(a.x, b, c.x), Vector8::NegMulAndAdd(a.y, b, c.y), Vector8::NegMulAndAdd(a.z, b, c.z) };
    }

    RT_FORCE_INLINE static const Vector3x8 NegMulAndSub(const Vector3x8& a, const Vector8& b, const Vector3x8& c)
    {
        return { Vector8::NegMulAndSub(a.x, b, c.x), Vector8::NegMulAndSub(a.y, b, c.y), Vector8::NegMulAndSub(a.z, b, c.z) };
    }

    //////////////////////////////////////////////////////////////////////////

    RT_FORCE_INLINE static const Vector3x8 MulAndAdd(const Vector3x8& a, const Vector3x8& b, const Vector8& c)
    {
        return { Vector8::MulAndAdd(a.x, b.x, c), Vector8::MulAndAdd(a.y, b.y, c), Vector8::MulAndAdd(a.z, b.z, c) };
    }

    RT_FORCE_INLINE static const Vector3x8 MulAndSub(const Vector3x8& a, const Vector3x8& b, const Vector8& c)
    {
        return { Vector8::MulAndSub(a.x, b.x, c), Vector8::MulAndSub(a.y, b.y, c), Vector8::MulAndSub(a.z, b.z, c) };
    }

    RT_FORCE_INLINE static const Vector3x8 NegMulAndAdd(const Vector3x8& a, const Vector3x8& b, const Vector8& c)
    {
        return { Vector8::NegMulAndAdd(a.x, b.x, c), Vector8::NegMulAndAdd(a.y, b.y, c), Vector8::NegMulAndAdd(a.z, b.z, c) };
    }

    RT_FORCE_INLINE static const Vector3x8 NegMulAndSub(const Vector3x8& a, const Vector3x8& b, const Vector8& c)
    {
        return { Vector8::NegMulAndSub(a.x, b.x, c), Vector8::NegMulAndSub(a.y, b.y, c), Vector8::NegMulAndSub(a.z, b.z, c) };
    }

    //////////////////////////////////////////////////////////////////////////

    RT_FORCE_INLINE static const Vector3x8 MulAndAdd(const Vector3x8& a, const Vector8& b, const Vector8& c)
    {
        return { Vector8::MulAndAdd(a.x, b, c), Vector8::MulAndAdd(a.y, b, c), Vector8::MulAndAdd(a.z, b, c) };
    }

    RT_FORCE_INLINE static const Vector3x8 MulAndSub(const Vector3x8& a, const Vector8& b, const Vector8& c)
    {
        return { Vector8::MulAndSub(a.x, b, c), Vector8::MulAndSub(a.y, b, c), Vector8::MulAndSub(a.z, b, c) };
    }

    RT_FORCE_INLINE static const Vector3x8 NegMulAndAdd(const Vector3x8& a, const Vector8& b, const Vector8& c)
    {
        return { Vector8::NegMulAndAdd(a.x, b, c), Vector8::NegMulAndAdd(a.y, b, c), Vector8::NegMulAndAdd(a.z, b, c) };
    }

    RT_FORCE_INLINE static const Vector3x8 NegMulAndSub(const Vector3x8& a, const Vector8& b, const Vector8& c)
    {
        return { Vector8::NegMulAndSub(a.x, b, c), Vector8::NegMulAndSub(a.y, b, c), Vector8::NegMulAndSub(a.z, b, c) };
    }

    //////////////////////////////////////////////////////////////////////////

    // 3D dot product
    RT_FORCE_INLINE static const Vector8 Dot(const Vector3x8& a, const Vector3x8& b)
    {
        // return a.x * b.x + a.y * b.y + a.z * b.z;
        return Vector8::MulAndAdd(a.x, b.x, Vector8::MulAndAdd(a.y, b.y, a.z * b.z));
    }

    // 3D cross product
    RT_FORCE_INLINE static const Vector3x8 Cross(const Vector3x8& a, const Vector3x8& b)
    {
        return {
            Vector8::NegMulAndAdd(a.z, b.y, a.y * b.z),
            Vector8::NegMulAndAdd(a.x, b.z, a.z * b.x),
            Vector8::NegMulAndAdd(a.y, b.x, a.x * b.y)
        };
    }

    RT_FORCE_INLINE const Vector8 SqrLength() const
    {
        return Dot(*this, *this);
    }

    RT_FORCE_INLINE const Vector3x8 Normalized() const
    {
        const Vector8 invLength = Vector8::Reciprocal(Vector8::Sqrt(SqrLength()));
        return (*this) * invLength;
    }

    //////////////////////////////////////////////////////////////////////////

    RT_FORCE_INLINE static const Vector3x8 Min(const Vector3x8& a, const Vector3x8& b)
    {
        return { Vector8::Min(a.x, b.x), Vector8::Min(a.y, b.y), Vector8::Min(a.z, b.z) };
    }

    RT_FORCE_INLINE static const Vector3x8 Max(const Vector3x8& a, const Vector3x8& b)
    {
        return { Vector8::Max(a.x, b.x), Vector8::Max(a.y, b.y), Vector8::Max(a.z, b.z) };
    }

};


} // namespace math
} // namespace rt
