#pragma once

namespace rt {
namespace math {

// Constructors ===================================================================================

Vector8::Vector8()
{
    v = _mm256_setzero_ps();
}

Vector8::Vector8(const Vector4& lo)
{
    v = _mm256_castps128_ps256(lo);
}

Vector8::Vector8(const Vector4& lo, const Vector4& hi)
{
    const __m256 tmp = _mm256_castps128_ps256(lo);
    v = _mm256_insertf128_ps(tmp, hi, 1);
}

Vector8::Vector8(const __m256& m)
    : v(m)
{
}

Vector8::Vector8(Float e0, Float e1, Float e2, Float e3, Float e4, Float e5, Float e6, Float e7)
{
    v = _mm256_set_ps(e0, e1, e2, e3, e4, e5, e6, e7);
}

Vector8::Vector8(Int32 e0, Int32 e1, Int32 e2, Int32 e3, Int32 e4, Int32 e5, Int32 e6, Int32 e7)
{
    v = _mm256_castsi256_ps(_mm256_set_epi32(e0, e1, e2, e3, e4, e5, e6, e7));
}

Vector8::Vector8(Uint32 e0, Uint32 e1, Uint32 e2, Uint32 e3, Uint32 e4, Uint32 e5, Uint32 e6, Uint32 e7)
{
    v = _mm256_castsi256_ps(_mm256_set_epi32(e0, e1, e2, e3, e4, e5, e6, e7));
}

Vector8::Vector8(const Float* src)
{
    v = _mm256_loadu_ps(src);
}

Vector8::Vector8(const Float scalar)
{
    v = _mm256_set1_ps(scalar);
}

Vector8::Vector8(const Int32 i)
{
	v = Vector8(_mm256_castsi256_ps(_mm256_set1_epi32(i)));
}

Vector8::Vector8(const Uint32 u)
{
	v = Vector8(_mm256_castsi256_ps(_mm256_set1_epi32(u)));
}

Vector8 Vector8::SelectBySign(const Vector8& a, const Vector8& b, const Vector8& sel)
{
    return _mm256_blendv_ps(a, b, sel);
}

template<Uint32 ix, Uint32 iy, Uint32 iz, Uint32 iw>
Vector8 Vector8::Swizzle() const
{
    static_assert(ix < 4, "Invalid X element index");
    static_assert(iy < 4, "Invalid Y element index");
    static_assert(iz < 4, "Invalid Z element index");
    static_assert(iw < 4, "Invalid W element index");

    return _mm256_shuffle_ps(v, v, _MM_SHUFFLE(iw, iz, iy, ix));
}

// Logical operations =============================================================================

Vector8 Vector8::operator& (const Vector8& b) const
{
    return _mm256_and_ps(v, b);
}

Vector8 Vector8::operator| (const Vector8& b) const
{
    return _mm256_or_ps(v, b);
}

Vector8 Vector8::operator^ (const Vector8& b) const
{
    return _mm256_xor_ps(v, b);
}

Vector8& Vector8::operator&= (const Vector8& b)
{
    v = _mm256_and_ps(v, b);
    return *this;
}

Vector8& Vector8::operator|= (const Vector8& b)
{
    v = _mm256_or_ps(v, b);
    return *this;
}

Vector8& Vector8::operator^= (const Vector8& b)
{
    v = _mm256_xor_ps(v, b);
    return *this;
}

// Simple arithmetics =============================================================================

Vector8 Vector8::operator- () const
{
    return Vector8() - (*this);
}

Vector8 Vector8::operator+ (const Vector8& b) const
{
    return _mm256_add_ps(v, b);
}

Vector8 Vector8::operator- (const Vector8& b) const
{
    return _mm256_sub_ps(v, b);
}

Vector8 Vector8::operator* (const Vector8& b) const
{
    return _mm256_mul_ps(v, b);
}

Vector8 Vector8::operator/ (const Vector8& b) const
{
    return _mm256_div_ps(v, b);
}

Vector8 Vector8::operator* (Float b) const
{
    return _mm256_mul_ps(v, _mm256_set1_ps(b));
}

Vector8 Vector8::operator/ (Float b) const
{
    return _mm256_div_ps(v, _mm256_set1_ps(b));
}

Vector8 operator*(Float a, const Vector8& b)
{
    return _mm256_mul_ps(b, _mm256_set1_ps(a));
}


Vector8& Vector8::operator+= (const Vector8& b)
{
    v = _mm256_add_ps(v, b);
    return *this;
}

Vector8& Vector8::operator-= (const Vector8& b)
{
    v = _mm256_sub_ps(v, b);
    return *this;
}

Vector8& Vector8::operator*= (const Vector8& b)
{
    v = _mm256_mul_ps(v, b);
    return *this;
}

Vector8& Vector8::operator/= (const Vector8& b)
{
    v = _mm256_div_ps(v, b);
    return *this;
}

Vector8& Vector8::operator*= (Float b)
{
    v = _mm256_mul_ps(v, _mm256_set1_ps(b));
    return *this;
}

Vector8& Vector8::operator/= (Float b)
{
    v = _mm256_div_ps(v, _mm256_set1_ps(b));
    return *this;
}

Vector8 Vector8::MulAndAdd(const Vector8& a, const Vector8& b, const Vector8& c)
{
#ifdef RT_USE_FMA
    return _mm256_fmadd_ps(a, b, c);
#else
    return a * b + c;
#endif
}

Vector8 Vector8::MulAndSub(const Vector8& a, const Vector8& b, const Vector8& c)
{
#ifdef RT_USE_FMA
    return _mm256_fmsub_ps(a, b, c);
#else
    return a * b - c;
#endif
}

Vector8 Vector8::NegMulAndAdd(const Vector8& a, const Vector8& b, const Vector8& c)
{
#ifdef RT_USE_FMA
    return _mm256_fnmadd_ps(a, b, c);
#else
    return -(a * b) + c;
#endif
}

Vector8 Vector8::NegMulAndSub(const Vector8& a, const Vector8& b, const Vector8& c)
{
#ifdef RT_USE_FMA
    return _mm256_fnmsub_ps(a, b, c);
#else
    return c - a * b;
#endif
}


Vector8 Vector8::Floor(const Vector8& V)
{
    Vector8 vResult = _mm256_sub_ps(V, _mm256_set1_ps(0.49999f));
    __m256i vInt = _mm256_cvtps_epi32(vResult);
    vResult = _mm256_cvtepi32_ps(vInt);
    return vResult;
}


Vector8 Vector8::Sqrt(const Vector8& V)
{
    return _mm256_sqrt_ps(V);
}

Vector8 Vector8::Reciprocal(const Vector8& V)
{
    return _mm256_div_ps(_mm256_set1_ps(1.0f), V);
}

Vector8 Vector8::FastReciprocal(const Vector8& v)
{
    const __m256 rcp = _mm256_rcp_ps(v);
    const __m256 rcpSqr = _mm256_mul_ps(rcp, rcp);
    const __m256 rcp2 = _mm256_add_ps(rcp, rcp);
    return _mm256_fnmadd_ps(rcpSqr, v, rcp2);
}

Vector8 Vector8::Lerp(const Vector8& v1, const Vector8& v2, const Vector8& weight)
{
    __m256 vTemp = _mm256_sub_ps(v2, v1);
    vTemp = _mm256_mul_ps(vTemp, weight);
    return _mm256_add_ps(v1, vTemp);
}

Vector8 Vector8::Lerp(const Vector8& v1, const Vector8& v2, Float weight)
{
    __m256 vWeight = _mm256_set1_ps(weight);
    __m256 vTemp = _mm256_sub_ps(v2, v1);
    vTemp = _mm256_mul_ps(vTemp, vWeight);
    return _mm256_add_ps(v1, vTemp);
}

Vector8 Vector8::Min(const Vector8& a, const Vector8& b)
{
    return _mm256_min_ps(a, b);
}

Vector8 Vector8::Max(const Vector8& a, const Vector8& b)
{
    return _mm256_max_ps(a, b);
}

Vector8 Vector8::Abs(const Vector8& v)
{
    return _mm256_and_ps(v, VECTOR8_MASK_ABS);
}

Vector8 Vector8::Clamped(const Vector8& min, const Vector8& max) const
{
    return Min(max, Max(min, *this));
}

Int32 Vector8::GetSignMask() const
{
    return _mm256_movemask_ps(v);
}

Vector8 Vector8::HorizontalMin() const
{
    __m256 temp;
    temp = _mm256_min_ps(v, _mm256_shuffle_ps(v, v, _MM_SHUFFLE(2, 3, 0, 1)));
    temp = _mm256_min_ps(temp, _mm256_shuffle_ps(temp, temp, _MM_SHUFFLE(1, 0, 3, 2)));
    temp = _mm256_min_ps(temp, _mm256_permute2f128_ps(temp, temp, 1));
    return temp;
}

Vector8 Vector8::HorizontalMax() const
{
    __m256 temp;
    temp = _mm256_max_ps(v, _mm256_shuffle_ps(v, v, _MM_SHUFFLE(2, 3, 0, 1)));
    temp = _mm256_max_ps(temp, _mm256_shuffle_ps(temp, temp, _MM_SHUFFLE(1, 0, 3, 2)));
    temp = _mm256_max_ps(temp, _mm256_permute2f128_ps(temp, temp, 1));
    return temp;
}

// Comparison functions ===========================================================================

Int32 Vector8::EqualMask(const Vector8& v1, const Vector8& v2)
{
    return _mm256_movemask_ps(_mm256_cmp_ps(v1, v2, _CMP_EQ_OQ));
}

Int32 Vector8::LessMask(const Vector8& v1, const Vector8& v2)
{
    return _mm256_movemask_ps(_mm256_cmp_ps(v1, v2, _CMP_LT_OQ));
}

Int32 Vector8::LessEqMask(const Vector8& v1, const Vector8& v2)
{
    return _mm256_movemask_ps(_mm256_cmp_ps(v1, v2, _CMP_LE_OQ));
}

Int32 Vector8::GreaterMask(const Vector8& v1, const Vector8& v2)
{
    return _mm256_movemask_ps(_mm256_cmp_ps(v1, v2, _CMP_GT_OQ));
}

Int32 Vector8::GreaterEqMask(const Vector8& v1, const Vector8& v2)
{
    return _mm256_movemask_ps(_mm256_cmp_ps(v1, v2, _CMP_GE_OQ));
}

Int32 Vector8::NotEqualMask(const Vector8& v1, const Vector8& v2)
{
    return _mm256_movemask_ps(_mm256_cmp_ps(v1, v2, _CMP_NEQ_OQ));
}

bool Vector8::operator== (const Vector8& b) const
{
    return EqualMask(*this, b) == 0xF;
}

bool Vector8::operator< (const Vector8& b) const
{
    return LessMask(*this, b) == 0xF;
}

bool Vector8::operator<= (const Vector8& b) const
{
    return LessEqMask(*this, b) == 0xF;
}

bool Vector8::operator> (const Vector8& b) const
{
    return GreaterMask(*this, b) == 0xF;
}

bool Vector8::operator>= (const Vector8& b) const
{
    return GreaterEqMask(*this, b) == 0xF;
}

bool Vector8::operator!= (const Vector8& b) const
{
    return NotEqualMask(*this, b) == 0xF;
}


} // namespace math
} // namespace rt
