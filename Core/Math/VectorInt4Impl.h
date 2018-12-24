#pragma once

#include "Vector4.h"

namespace rt {
namespace math {


const VectorInt4 VectorInt4::Zero()
{
    return _mm_setzero_si128();
}

VectorInt4::VectorInt4(const __m128i& m)
    : v(m)
{}

VectorInt4::VectorInt4(const VectorInt4& other)
    : v(other.v)
{}

const Vector4 VectorInt4::CastToFloat() const
{
    return _mm_castsi128_ps(v);
}

VectorInt4::VectorInt4(const Int32 x, const Int32 y, const Int32 z, const Int32 w)
    : v(_mm_set_epi32(w, z, y, x))
{}

VectorInt4::VectorInt4(const Int32 i)
    : v(_mm_set1_epi32(i))
{}

VectorInt4::VectorInt4(const Uint32 u)
    : v(_mm_set1_epi32(u))
{}

const VectorInt4 VectorInt4::Convert(const Vector4& v)
{
    return _mm_cvtps_epi32(v);
}

const Vector4 VectorInt4::ConvertToFloat() const
{
    return _mm_cvtepi32_ps(v);
}

//////////////////////////////////////////////////////////////////////////

template<Uint32 ix, Uint32 iy, Uint32 iz, Uint32 iw>
const VectorInt4 VectorInt4::Swizzle() const
{
    static_assert(ix < 4, "Invalid X element index");
    static_assert(iy < 4, "Invalid Y element index");
    static_assert(iz < 4, "Invalid Z element index");
    static_assert(iw < 4, "Invalid W element index");

    return _mm_shuffle_epi32(v, _MM_SHUFFLE(iw, iz, iy, ix));
}

//////////////////////////////////////////////////////////////////////////

const VectorInt4 VectorInt4::operator & (const VectorInt4& b) const
{
    return VectorInt4(_mm_and_si128(v, b.v));
}

const VectorInt4 VectorInt4::operator | (const VectorInt4& b) const
{
    return VectorInt4(_mm_or_si128(v, b.v));
}

const VectorInt4 VectorInt4::operator ^ (const VectorInt4& b) const
{
    return VectorInt4(_mm_xor_si128(v, b.v));
}

VectorInt4& VectorInt4::operator &= (const VectorInt4& b)
{
    v = _mm_and_si128(v, b.v);
    return *this;
}

VectorInt4& VectorInt4::operator |= (const VectorInt4& b)
{
    v = _mm_or_si128(v, b.v);
    return *this;
}

VectorInt4& VectorInt4::operator ^= (const VectorInt4& b)
{
    v = _mm_xor_si128(v, b.v);
    return *this;
}

//////////////////////////////////////////////////////////////////////////

const VectorInt4 VectorInt4::operator - () const
{
    return VectorInt4::Zero() - (*this);
}

const VectorInt4 VectorInt4::operator + (const VectorInt4& b) const
{
    return _mm_add_epi32(v, b);
}

const VectorInt4 VectorInt4::operator - (const VectorInt4& b) const
{
    return _mm_sub_epi32(v, b);
}

const VectorInt4 VectorInt4::operator * (const VectorInt4& b) const
{
    return _mm_mullo_epi32(v, b);
}

VectorInt4& VectorInt4::operator += (const VectorInt4& b)
{
    v = _mm_add_epi32(v, b);
    return *this;
}

VectorInt4& VectorInt4::operator -= (const VectorInt4& b)
{
    v = _mm_sub_epi32(v, b);
    return *this;
}

VectorInt4& VectorInt4::operator *= (const VectorInt4& b)
{
    v = _mm_mullo_epi32(v, b);
    return *this;
}

const VectorInt4 VectorInt4::operator + (Int32 b) const
{
    return _mm_add_epi32(v, _mm_set1_epi32(b));
}

const VectorInt4 VectorInt4::operator - (Int32 b) const
{
    return _mm_sub_epi32(v, _mm_set1_epi32(b));
}

const VectorInt4 VectorInt4::operator * (Int32 b) const
{
    return _mm_mullo_epi32(v, _mm_set1_epi32(b));
}

VectorInt4& VectorInt4::operator += (Int32 b)
{
    v = _mm_add_epi32(v, _mm_set1_epi32(b));
    return *this;
}

VectorInt4& VectorInt4::operator -= (Int32 b)
{
    v = _mm_sub_epi32(v, _mm_set1_epi32(b));
    return *this;
}

VectorInt4& VectorInt4::operator *= (Int32 b)
{
    v = _mm_mullo_epi32(v, _mm_set1_epi32(b));
    return *this;
}

//////////////////////////////////////////////////////////////////////////

const VectorInt4 VectorInt4::operator << (Int32 b) const
{
    return _mm_slli_epi32(v, b);
}

const VectorInt4 VectorInt4::operator >> (Int32 b) const
{
    return _mm_srli_epi32(v, b);
}

//////////////////////////////////////////////////////////////////////////

const VectorInt4 VectorInt4::SetIfGreaterOrEqual(const VectorInt4& reference, const VectorInt4& target) const
{
    const __m128i mask = _mm_cmplt_epi32(v, reference.v);
    return VectorInt4(_mm_blendv_epi8(target, v, mask));
}

bool VectorInt4::operator == (const VectorInt4& b) const
{
    return _mm_movemask_ps(_mm_cvtepi32_ps(_mm_cmpeq_epi32(v, b.v))) == 0xF;
}

bool VectorInt4::operator != (const VectorInt4& b) const
{
    return _mm_movemask_ps(_mm_cvtepi32_ps(_mm_cmpeq_epi32(v, b.v))) != 0xF;
}

//////////////////////////////////////////////////////////////////////////

const VectorInt4 VectorInt4::Min(const VectorInt4& a, const VectorInt4& b)
{
    return _mm_min_epi32(a, b);
}

const VectorInt4 VectorInt4::Max(const VectorInt4& a, const VectorInt4& b)
{
    return _mm_max_epi32(a, b);
}


} // namespace math
} // namespace rt
