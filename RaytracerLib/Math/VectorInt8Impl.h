#pragma once

#include "Vector8.h"

namespace rt {
namespace math {


VectorInt8::VectorInt8()
{
    v = _mm256_setzero_si256();
}

VectorInt8::VectorInt8(const __m256i& m)
    : v(m)
{}

VectorInt8::VectorInt8(const __m256& m)
    : f(m)
{}

VectorInt8::VectorInt8(const Vector8& v)
    : f(v)
{}

VectorInt8::VectorInt8(const Int32 e0, const Int32 e1, const Int32 e2, const Int32 e3, const Int32 e4, const Int32 e5, const Int32 e6, const Int32 e7)
{
    v = _mm256_set_epi32(e0, e1, e2, e3, e4, e5, e6, e7);
}

VectorInt8::VectorInt8(const Int32 i)
{
	v = _mm256_set1_epi32(i);
}

VectorInt8::VectorInt8(const Uint32 u)
{
	v = _mm256_set1_epi32(u);
}

VectorInt8 VectorInt8::SelectBySign(const VectorInt8& a, const VectorInt8& b, const VectorInt8& sel)
{
    return VectorInt8(_mm256_blendv_ps(a.f, b.f, sel.f));
}

VectorInt8 VectorInt8::operator& (const VectorInt8& b) const
{
    return _mm256_and_si256(v, b);
}

VectorInt8 VectorInt8::operator| (const VectorInt8& b) const
{
    return _mm256_or_si256(v, b);
}

VectorInt8 VectorInt8::operator^ (const VectorInt8& b) const
{
    return _mm256_xor_si256(v, b);
}

VectorInt8& VectorInt8::operator&= (const VectorInt8& b)
{
    v = _mm256_and_si256(v, b);
    return *this;
}

VectorInt8& VectorInt8::operator|= (const VectorInt8& b)
{
    v = _mm256_or_si256(v, b);
    return *this;
}

VectorInt8& VectorInt8::operator^= (const VectorInt8& b)
{
    v = _mm256_xor_si256(v, b);
    return *this;
}

VectorInt8 VectorInt8::operator- () const
{
    return VectorInt8() - (*this);
}

VectorInt8 VectorInt8::operator+ (const VectorInt8& b) const
{
    return _mm256_add_epi32(v, b);
}

VectorInt8 VectorInt8::operator- (const VectorInt8& b) const
{
    return _mm256_sub_epi32(v, b);
}

VectorInt8& VectorInt8::operator+= (const VectorInt8& b)
{
    v = _mm256_add_epi32(v, b);
    return *this;
}

VectorInt8& VectorInt8::operator-= (const VectorInt8& b)
{
    v = _mm256_sub_epi32(v, b);
    return *this;
}

VectorInt8 VectorInt8::Min(const VectorInt8& a, const VectorInt8& b)
{
    return _mm256_min_epi32(a, b);
}

VectorInt8 VectorInt8::Max(const VectorInt8& a, const VectorInt8& b)
{
    return _mm256_max_epi32(a, b);
}

} // namespace math
} // namespace rt
