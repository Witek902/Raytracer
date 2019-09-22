#pragma once

namespace rt {
namespace math {

// Constructors ===================================================================================

Vector8::Vector8() = default;

const Vector8 Vector8::Zero()
{
    return _mm256_setzero_ps();
}

Vector8::Vector8(const Vector8& other)
    : v(other.v)
{}

Vector8::Vector8(const Vector4& lo)
    : v(_mm256_castps128_ps256(lo))
{}

Vector8::Vector8(const Vector4& lo, const Vector4& hi)
    : v(_mm256_insertf128_ps(_mm256_castps128_ps256(lo), hi, 1))
{}

Vector8::Vector8(const __m256& m)
    : v(m)
{}

Vector8::Vector8(float e0, float e1, float e2, float e3, float e4, float e5, float e6, float e7)
    : v(_mm256_set_ps(e7, e6, e5, e4, e3, e2, e1, e0))
{}

Vector8::Vector8(int32 e0, int32 e1, int32 e2, int32 e3, int32 e4, int32 e5, int32 e6, int32 e7)
    : v(_mm256_castsi256_ps(_mm256_set_epi32(e7, e6, e5, e4, e3, e2, e1, e0)))
{}

Vector8::Vector8(uint32 e0, uint32 e1, uint32 e2, uint32 e3, uint32 e4, uint32 e5, uint32 e6, uint32 e7)
    : v(_mm256_castsi256_ps(_mm256_set_epi32(e7, e6, e5, e4, e3, e2, e1, e0)))
{}

Vector8::Vector8(const float* src)
    : v(_mm256_loadu_ps(src))
{}

Vector8::Vector8(const float scalar)
    : v(_mm256_set1_ps(scalar))
{}

Vector8::Vector8(const int32 i)
    : v(_mm256_castsi256_ps(_mm256_set1_epi32(i)))
{}

Vector8::Vector8(const uint32 u)
    : v(_mm256_castsi256_ps(_mm256_set1_epi32(u)))
{}

const Vector8 Vector8::FromInteger(int32 x)
{
    return _mm256_cvtepi32_ps(_mm256_set1_epi32(x));
}

Vector8& Vector8::operator = (const Vector8& other)
{
    v = other.v;
    return *this;
}

const Vector8 Vector8::Select(const Vector8& a, const Vector8& b, const VectorBool8& sel)
{
    return _mm256_blendv_ps(a, b, sel.v);
}

template<uint32 selX, uint32 selY, uint32 selZ, uint32 selW>
const Vector8 Vector8::Select(const Vector8& a, const Vector8& b)
{
    static_assert(selX <= 1, "Invalid X index");
    static_assert(selY <= 1, "Invalid Y index");
    static_assert(selZ <= 1, "Invalid Z index");
    static_assert(selW <= 1, "Invalid W index");

    constexpr uint32 maskLow = selX | (selY << 1) | (selZ << 2) | (selW << 3);
    return _mm256_blend_ps(a, b, maskLow | (maskLow << 4));
}

template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
const Vector8 Vector8::Swizzle() const
{
    static_assert(ix < 4, "Invalid X element index");
    static_assert(iy < 4, "Invalid Y element index");
    static_assert(iz < 4, "Invalid Z element index");
    static_assert(iw < 4, "Invalid W element index");

    return _mm256_shuffle_ps(v, v, _MM_SHUFFLE(iw, iz, iy, ix));
}

// extract lower lanes
const Vector4 Vector8::Low() const
{
    return Vector4(_mm256_extractf128_ps(v, 0));
}

// extract higher lanes
const Vector4 Vector8::High() const
{
    return Vector4(_mm256_extractf128_ps(v, 1));
}

// Logical operations =============================================================================

const Vector8 Vector8::operator& (const Vector8& b) const
{
    return _mm256_and_ps(v, b);
}

const Vector8 Vector8::operator| (const Vector8& b) const
{
    return _mm256_or_ps(v, b);
}

const Vector8 Vector8::operator^ (const Vector8& b) const
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

const Vector8 Vector8::operator- () const
{
    return Vector8::Zero() - (*this);
}

const Vector8 Vector8::operator+ (const Vector8& b) const
{
    return _mm256_add_ps(v, b);
}

const Vector8 Vector8::operator- (const Vector8& b) const
{
    return _mm256_sub_ps(v, b);
}

const Vector8 Vector8::operator* (const Vector8& b) const
{
    return _mm256_mul_ps(v, b);
}

const Vector8 Vector8::operator/ (const Vector8& b) const
{
    return _mm256_div_ps(v, b);
}

const Vector8 Vector8::operator* (float b) const
{
    return _mm256_mul_ps(v, _mm256_set1_ps(b));
}

const Vector8 Vector8::operator/ (float b) const
{
    return _mm256_div_ps(v, _mm256_set1_ps(b));
}

const Vector8 operator*(float a, const Vector8& b)
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

Vector8& Vector8::operator*= (float b)
{
    v = _mm256_mul_ps(v, _mm256_set1_ps(b));
    return *this;
}

Vector8& Vector8::operator/= (float b)
{
    v = _mm256_div_ps(v, _mm256_set1_ps(b));
    return *this;
}

const Vector8 Vector8::MulAndAdd(const Vector8& a, const Vector8& b, const Vector8& c)
{
#ifdef RT_USE_FMA
    return _mm256_fmadd_ps(a, b, c);
#else
    return a * b + c;
#endif
}

const Vector8 Vector8::MulAndSub(const Vector8& a, const Vector8& b, const Vector8& c)
{
#ifdef RT_USE_FMA
    return _mm256_fmsub_ps(a, b, c);
#else
    return a * b - c;
#endif
}

const Vector8 Vector8::NegMulAndAdd(const Vector8& a, const Vector8& b, const Vector8& c)
{
#ifdef RT_USE_FMA
    return _mm256_fnmadd_ps(a, b, c);
#else
    return -(a * b) + c;
#endif
}

const Vector8 Vector8::NegMulAndSub(const Vector8& a, const Vector8& b, const Vector8& c)
{
#ifdef RT_USE_FMA
    return _mm256_fnmsub_ps(a, b, c);
#else
    return c - a * b;
#endif
}

const Vector8 Vector8::Floor(const Vector8& V)
{
    Vector8 vResult = _mm256_sub_ps(V, _mm256_set1_ps(0.49999f));
    __m256i vInt = _mm256_cvtps_epi32(vResult);
    vResult = _mm256_cvtepi32_ps(vInt);
    return vResult;
}

const Vector8 Vector8::Sqrt(const Vector8& V)
{
    return _mm256_sqrt_ps(V);
}

const Vector8 Vector8::Reciprocal(const Vector8& V)
{
    return _mm256_div_ps(_mm256_set1_ps(1.0f), V);
}

const Vector8 Vector8::FastReciprocal(const Vector8& v)
{
    const __m256 rcp = _mm256_rcp_ps(v);
    const __m256 rcpSqr = _mm256_mul_ps(rcp, rcp);
    const __m256 rcp2 = _mm256_add_ps(rcp, rcp);
    return NegMulAndAdd(rcpSqr, v, rcp2);
}

const Vector8 Vector8::Min(const Vector8& a, const Vector8& b)
{
    return _mm256_min_ps(a, b);
}

const Vector8 Vector8::Max(const Vector8& a, const Vector8& b)
{
    return _mm256_max_ps(a, b);
}

const Vector8 Vector8::Abs(const Vector8& v)
{
    return _mm256_and_ps(v, VECTOR8_MASK_ABS);
}

int32 Vector8::GetSignMask() const
{
    return _mm256_movemask_ps(v);
}

const Vector8 Vector8::HorizontalMax() const
{
    __m256 temp;
    temp = _mm256_max_ps(v, _mm256_shuffle_ps(v, v, _MM_SHUFFLE(2, 3, 0, 1)));
    temp = _mm256_max_ps(temp, _mm256_shuffle_ps(temp, temp, _MM_SHUFFLE(1, 0, 3, 2)));
    temp = _mm256_max_ps(temp, _mm256_permute2f128_ps(temp, temp, 1));
    return temp;
}

const Vector8 Vector8::Fmod1(const Vector8& x)
{
    return _mm256_sub_ps(x, _mm256_round_ps(x, _MM_FROUND_TO_ZERO));
}

/*
void Vector8::Transpose8x8(Vector8& v0, Vector8& v1, Vector8& v2, Vector8& v3, Vector8& v4, Vector8& v5, Vector8& v6, Vector8& v7)
{
    const __m256 t0 = _mm256_unpacklo_ps(v0, v1);
    const __m256 t1 = _mm256_unpackhi_ps(v0, v1);
    const __m256 t2 = _mm256_unpacklo_ps(v2, v3);
    const __m256 t3 = _mm256_unpackhi_ps(v2, v3);
    const __m256 t4 = _mm256_unpacklo_ps(v4, v5);
    const __m256 t5 = _mm256_unpackhi_ps(v4, v5);
    const __m256 t6 = _mm256_unpacklo_ps(v6, v7);
    const __m256 t7 = _mm256_unpackhi_ps(v6, v7);

    // Using 4 shuffles + 8 blends (12 instructions in total) is faster than only 8 shuffles
    // blends can be executed in parallel with shuffles, while shuffle can be only executed at port 5
    __m256 v;
    v = _mm256_shuffle_ps(t0, t2, 0x4E);
    const __m256 tt0 = _mm256_blend_ps(t0, v, 0xCC);
    const __m256 tt1 = _mm256_blend_ps(t2, v, 0x33);
    v = _mm256_shuffle_ps(t1, t3, 0x4E);
    const __m256 tt2 = _mm256_blend_ps(t1, v, 0xCC);
    const __m256 tt3 = _mm256_blend_ps(t3, v, 0x33);
    v = _mm256_shuffle_ps(t4, t6, 0x4E);
    const __m256 tt4 = _mm256_blend_ps(t4, v, 0xCC);
    const __m256 tt5 = _mm256_blend_ps(t6, v, 0x33);
    v = _mm256_shuffle_ps(t5, t7, 0x4E);
    const __m256 tt6 = _mm256_blend_ps(t5, v, 0xCC);
    const __m256 tt7 = _mm256_blend_ps(t7, v, 0x33);

    //const __m256 tt0 = _mm256_shuffle_ps(t0, t2, _MM_SHUFFLE(1, 0, 1, 0));
    //const __m256 tt1 = _mm256_shuffle_ps(t0, t2, _MM_SHUFFLE(3, 2, 3, 2));
    //const __m256 tt2 = _mm256_shuffle_ps(t1, t3, _MM_SHUFFLE(1, 0, 1, 0));
    //const __m256 tt3 = _mm256_shuffle_ps(t1, t3, _MM_SHUFFLE(3, 2, 3, 2));
    //const __m256 tt4 = _mm256_shuffle_ps(t4, t6, _MM_SHUFFLE(1, 0, 1, 0));
    //const __m256 tt5 = _mm256_shuffle_ps(t4, t6, _MM_SHUFFLE(3, 2, 3, 2));
    //const __m256 tt6 = _mm256_shuffle_ps(t5, t7, _MM_SHUFFLE(1, 0, 1, 0));
    //const __m256 tt7 = _mm256_shuffle_ps(t5, t7, _MM_SHUFFLE(3, 2, 3, 2));

    v0 = _mm256_permute2f128_ps(tt0, tt4, 0x20);
    v1 = _mm256_permute2f128_ps(tt1, tt5, 0x20);
    v2 = _mm256_permute2f128_ps(tt2, tt6, 0x20);
    v3 = _mm256_permute2f128_ps(tt3, tt7, 0x20);
    v4 = _mm256_permute2f128_ps(tt0, tt4, 0x31);
    v5 = _mm256_permute2f128_ps(tt1, tt5, 0x31);
    v6 = _mm256_permute2f128_ps(tt2, tt6, 0x31);
    v7 = _mm256_permute2f128_ps(tt3, tt7, 0x31);
}
*/

// Comparison functions ===========================================================================

const VectorBool8 Vector8::operator == (const Vector8& b) const
{
    return _mm256_cmp_ps(v, b.v, _CMP_EQ_OQ);
}

const VectorBool8 Vector8::operator < (const Vector8& b) const
{
    return _mm256_cmp_ps(v, b.v, _CMP_LT_OQ);
}

const VectorBool8 Vector8::operator <= (const Vector8& b) const
{
    return _mm256_cmp_ps(v, b.v, _CMP_LE_OQ);
}

const VectorBool8 Vector8::operator > (const Vector8& b) const
{
    return _mm256_cmp_ps(v, b.v, _CMP_GT_OQ);
}

const VectorBool8 Vector8::operator >= (const Vector8& b) const
{
    return _mm256_cmp_ps(v, b.v, _CMP_GE_OQ);
}

const VectorBool8 Vector8::operator != (const Vector8& b) const
{
    return _mm256_cmp_ps(v, b.v, _CMP_NEQ_OQ);
}

bool Vector8::IsZero() const
{
    return _mm256_movemask_ps(_mm256_cmp_ps(v, _mm256_setzero_ps(), _CMP_EQ_OQ)) == 0xFF;
}

bool Vector8::IsNaN() const
{
    // Test against itself. NaN is always not equal
    const __m256 temp = _mm256_cmp_ps(v, v, _CMP_NEQ_OQ);
    return _mm256_movemask_ps(temp) != 0;
}

bool Vector8::IsInfinite() const
{
    // Mask off the sign bit
    __m256 temp = _mm256_and_ps(v, VECTOR8_MASK_ABS);
    // Compare to infinity
    temp = _mm256_cmp_ps(temp, VECTOR8_INF, _CMP_EQ_OQ);
    return _mm256_movemask_ps(temp) != 0;
}

} // namespace math
} // namespace rt
