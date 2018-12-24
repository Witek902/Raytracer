#pragma once

#include "Vector8.h"

namespace rt {
namespace math {


const VectorInt8 VectorInt8::Zero()
{
    return _mm256_setzero_si256();
}

VectorInt8::VectorInt8(const __m256i& m)
    : v(m)
{}

VectorInt8::VectorInt8(const __m256& m)
    : f(m)
{}

VectorInt8::VectorInt8(const VectorInt4& lo, const VectorInt4& hi)
    : v(_mm256_insertf128_si256(_mm256_castsi128_si256(hi), lo, 1u))
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

#ifdef RT_USE_AVX2

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
    return VectorInt8() - (*this);
}

const VectorInt8 VectorInt8::operator + (const VectorInt8& b) const
{
    return _mm256_add_epi32(v, b);
}

const VectorInt8 VectorInt8::operator - (const VectorInt8& b) const
{
    return _mm256_sub_epi32(v, b);
}

VectorInt8& VectorInt8::operator += (const VectorInt8& b)
{
    v = _mm256_add_epi32(v, b);
    return *this;
}

VectorInt8& VectorInt8::operator -= (const VectorInt8& b)
{
    v = _mm256_sub_epi32(v, b);
    return *this;
}

const VectorInt8 VectorInt8::operator + (Int32 b) const
{
    return _mm256_add_epi32(v, _mm256_set1_epi32(b));
}

const VectorInt8 VectorInt8::operator - (Int32 b) const
{
    return _mm256_sub_epi32(v, _mm256_set1_epi32(b));
}

VectorInt8& VectorInt8::operator += (Int32 b)
{
    v = _mm256_add_epi32(v, _mm256_set1_epi32(b));
    return *this;
}

VectorInt8& VectorInt8::operator -= (Int32 b)
{
    v = _mm256_sub_epi32(v, _mm256_set1_epi32(b));
    return *this;
}

//////////////////////////////////////////////////////////////////////////

const VectorInt8 VectorInt8::operator << (Int32 b) const
{
    return _mm256_slli_epi32(v, b);
}

const VectorInt8 VectorInt8::operator >> (Int32 b) const
{
    return _mm256_srli_epi32(v, b);
}

//////////////////////////////////////////////////////////////////////////

const VectorInt8 VectorInt8::Min(const VectorInt8& a, const VectorInt8& b)
{
    return _mm256_min_epi32(a, b);
}

const VectorInt8 VectorInt8::Max(const VectorInt8& a, const VectorInt8& b)
{
    return _mm256_max_epi32(a, b);
}

#endif // RT_USE_AVX2

} // namespace math
} // namespace rt
