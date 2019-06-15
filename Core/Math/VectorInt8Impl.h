#pragma once

#include "Vector8.h"

namespace rt {
namespace math {

const VectorInt8 VectorInt8::Zero()
{
#ifdef RT_USE_AVX2
    return _mm256_setzero_si256();
#elif defined(RT_USE_AVX)
    return VectorInt8(_mm256_setzero_ps());
#else
    return VectorInt8{ 0 };
#endif
}

VectorInt8::VectorInt8(const VectorInt8& other)
    : v(other.v)
{}

VectorInt8& VectorInt8::operator = (const VectorInt8& other)
{
    v = other.v;
    return *this;
}

VectorInt8::VectorInt8(const __m256i& m)
    : v(m)
{}

VectorInt8::VectorInt8(const __m256& m)
    : f(m)
{}

VectorInt8::VectorInt8(const VectorInt4& lo, const VectorInt4& hi)
    : v(_mm256_insertf128_si256(_mm256_castsi128_si256(lo), hi, 1u))
{}

const VectorInt8 VectorInt8::Cast(const Vector8& v)
{
    return _mm256_castps_si256(v);
}

const Vector8 VectorInt8::CastToFloat() const
{
    return _mm256_castsi256_ps(v);
}

VectorInt8::VectorInt8(const Int32 e0, const Int32 e1, const Int32 e2, const Int32 e3, const Int32 e4, const Int32 e5, const Int32 e6, const Int32 e7)
    : v(_mm256_set_epi32(e7, e6, e5, e4, e3, e2, e1, e0))
{}

VectorInt8::VectorInt8(const Int32 i)
    : v(_mm256_set1_epi32(i))
{}

VectorInt8::VectorInt8(const Uint32 u)
    : v(_mm256_set1_epi32(u))
{}

const VectorInt8 VectorInt8::SelectBySign(const VectorInt8& a, const VectorInt8& b, const VectorInt8& sel)
{
    return VectorInt8(_mm256_blendv_ps(a.f, b.f, sel.f));
}

const VectorInt8 VectorInt8::operator & (const VectorInt8& b) const
{
    return VectorInt8(_mm256_and_ps(f, b.f));
}

const VectorInt8 VectorInt8::operator | (const VectorInt8& b) const
{
    return VectorInt8(_mm256_or_ps(f, b.f));
}

const VectorInt8 VectorInt8::operator ^ (const VectorInt8& b) const
{
    return VectorInt8(_mm256_xor_ps(f, b.f));
}

VectorInt8& VectorInt8::operator &= (const VectorInt8& b)
{
    f = _mm256_and_ps(f, b.f);
    return *this;
}

VectorInt8& VectorInt8::operator |= (const VectorInt8& b)
{
    f = _mm256_or_ps(f, b.f);
    return *this;
}

VectorInt8& VectorInt8::operator ^= (const VectorInt8& b)
{
    f = _mm256_xor_ps(f, b.f);
    return *this;
}

const VectorInt8 VectorInt8::Convert(const Vector8& v)
{
    return _mm256_cvtps_epi32(_mm256_round_ps(v, _MM_FROUND_TO_ZERO));
}

const Vector8 VectorInt8::ConvertToFloat() const
{
    return _mm256_cvtepi32_ps(v);
}

const VectorInt8 VectorInt8::operator - () const
{
    return VectorInt8::Zero() - (*this);
}

const VectorInt8 VectorInt8::operator + (const VectorInt8& b) const
{
#ifdef RT_USE_AVX2
    return _mm256_add_epi32(v, b);
#else
    return { _mm_add_epi32(low, b.low), _mm_add_epi32(high, b.high) };
#endif
}

const VectorInt8 VectorInt8::operator - (const VectorInt8& b) const
{
#ifdef RT_USE_AVX2
    return _mm256_sub_epi32(v, b);
#else
    return { _mm_sub_epi32(low, b.low), _mm_sub_epi32(high, b.high) };
#endif
}

const VectorInt8 VectorInt8::operator * (const VectorInt8& b) const
{
#ifdef RT_USE_AVX2
    return _mm256_mullo_epi32(v, b);
#else
    return { _mm_mullo_epi32(low, b.low), _mm_mullo_epi32(high, b.high) };
#endif
}

VectorInt8& VectorInt8::operator += (const VectorInt8& b)
{
#ifdef RT_USE_AVX2
    v = _mm256_add_epi32(v, b);
#else
    low = _mm_add_epi32(low, b.low);
    high = _mm_add_epi32(high, b.high);
#endif
    return *this;
}

VectorInt8& VectorInt8::operator -= (const VectorInt8& b)
{
#ifdef RT_USE_AVX2
    v = _mm256_sub_epi32(v, b);
#else
    low = _mm_sub_epi32(low, b.low);
    high = _mm_sub_epi32(high, b.high);
#endif
    return *this;
}

VectorInt8& VectorInt8::operator *= (const VectorInt8& b)
{
#ifdef RT_USE_AVX2
    v = _mm256_mullo_epi32(v, b);
#else
    low = _mm_mullo_epi32(low, b.low);
    high = _mm_mullo_epi32(high, b.high);
#endif
    return *this;
}

const VectorInt8 VectorInt8::operator + (Int32 b) const
{
#ifdef RT_USE_AVX2
    return _mm256_add_epi32(v, _mm256_set1_epi32(b));
#else
    const __m128i temp = _mm_set1_epi32(b);
    return { _mm_add_epi32(low, temp), _mm_add_epi32(high, temp) };
#endif
}

const VectorInt8 VectorInt8::operator - (Int32 b) const
{
#ifdef RT_USE_AVX2
    return _mm256_sub_epi32(v, _mm256_set1_epi32(b));
#else
    const __m128i temp = _mm_set1_epi32(b);
    return { _mm_sub_epi32(low, temp), _mm_sub_epi32(high, temp) };
#endif
}

const VectorInt8 VectorInt8::operator * (Int32 b) const
{
#ifdef RT_USE_AVX2
    return _mm256_mullo_epi32(v, _mm256_set1_epi32(b));
#else
    const __m128i temp = _mm_set1_epi32(b);
    return { _mm_mullo_epi32(low, temp), _mm_mullo_epi32(high, temp) };
#endif
}

const VectorInt8 VectorInt8::operator % (Int32 b) const
{
    // TODO
    return VectorInt8(i[0] % b, i[1] % b, i[2] % b, i[3] % b, i[4] % b, i[5] % b, i[6] % b, i[7] % b);
}

VectorInt8& VectorInt8::operator += (Int32 b)
{
#ifdef RT_USE_AVX2
    v = _mm256_add_epi32(v, _mm256_set1_epi32(b));
#else
    const __m128i temp = _mm_set1_epi32(b);
    low = _mm_add_epi32(low, temp);
    high = _mm_add_epi32(high, temp);
#endif
    return *this;
}

VectorInt8& VectorInt8::operator -= (Int32 b)
{
#ifdef RT_USE_AVX2
    v = _mm256_sub_epi32(v, _mm256_set1_epi32(b));
#else
    const __m128i temp = _mm_set1_epi32(b);
    low = _mm_sub_epi32(low, temp);
    high = _mm_sub_epi32(high, temp);
#endif
    return *this;
}

VectorInt8& VectorInt8::operator *= (Int32 b)
{
#ifdef RT_USE_AVX2
    v = _mm256_mullo_epi32(v, _mm256_set1_epi32(b));
#else
    const __m128i temp = _mm_set1_epi32(b);
    low = _mm_mullo_epi32(low, temp);
    high = _mm_mullo_epi32(high, temp);
#endif
    return *this;
}

//////////////////////////////////////////////////////////////////////////

bool VectorInt8::operator == (const VectorInt8& b) const
{
#ifdef RT_USE_AVX2
    return _mm256_movemask_ps(_mm256_cvtepi32_ps(_mm256_cmpeq_epi32(v, b.v))) == 0xFF;
#else
    return _mm_test_all_ones(_mm_cmpeq_epi32(low, b.low)) && _mm_test_all_ones(_mm_cmpeq_epi32(high, b.high));
#endif
}

bool VectorInt8::operator != (const VectorInt8& b) const
{
#ifdef RT_USE_AVX2
    return _mm256_movemask_ps(_mm256_cvtepi32_ps(_mm256_cmpeq_epi32(v, b.v))) != 0xFF;
#else
    return !operator==(b);
#endif
}

//////////////////////////////////////////////////////////////////////////

const VectorInt8 VectorInt8::operator << (Int32 b) const
{
#ifdef RT_USE_AVX2
    return _mm256_slli_epi32(v, b);
#else
    return { _mm_slli_epi32(low, b), _mm_slli_epi32(high, b) };
#endif
}

const VectorInt8 VectorInt8::operator >> (Int32 b) const
{
#ifdef RT_USE_AVX2
    return _mm256_srli_epi32(v, b);
#else
    return { _mm_srli_epi32(low, b), _mm_srli_epi32(high, b) };
#endif
}

//////////////////////////////////////////////////////////////////////////

const VectorInt8 VectorInt8::Min(const VectorInt8& a, const VectorInt8& b)
{
#ifdef RT_USE_AVX2
    return _mm256_min_epi32(a, b);
#else
    return { _mm_min_epi32(a.low, b.low), _mm_min_epi32(a.high, b.high) };
#endif
}

const VectorInt8 VectorInt8::Max(const VectorInt8& a, const VectorInt8& b)
{
#ifdef RT_USE_AVX2
    return _mm256_max_epi32(a, b);
#else
    return { _mm_max_epi32(a.low, b.low), _mm_max_epi32(a.high, b.high) };
#endif
}

const Vector8 Gather8(const float* basePtr, const VectorInt8& indices)
{
#ifdef RT_USE_AVX2
    return _mm256_i32gather_ps(basePtr, indices, 4);
#else
    Vector8 result;
    for (Uint32 i = 0; i < 8; ++i)
    {
        result[i] = basePtr[indices[i]];
    }
    return result;
#endif
}

} // namespace math
} // namespace rt
