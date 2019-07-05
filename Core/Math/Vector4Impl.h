#pragma once

#include "Half.h"

namespace rt {
namespace math {

// Constructors ===================================================================================

const Vector4 Vector4::Zero()
{
    return _mm_setzero_ps();
}

#ifdef _DEBUG
Vector4::Vector4()
    : v(_mm_set1_ps(std::numeric_limits<float>::signaling_NaN()))
{}
#else
Vector4::Vector4() = default;
#endif // _DEBUG

Vector4::Vector4(const Vector4& other)
    : v(other.v)
{}

Vector4::Vector4(const __m128& src)
    : v(src)
{ }

Vector4::Vector4(const float s)
    : v(_mm_set1_ps(s))
{}

Vector4::Vector4(const float x, const float y, const float z = 0.0f, const float w = 0.0f)
    : v(_mm_set_ps(w, z, y, x))
{}

Vector4::Vector4(const Int32 x, const Int32 y, const Int32 z = 0, const Int32 w = 0)
    : v(_mm_castsi128_ps(_mm_set_epi32(w, z, y, x)))
{}

Vector4::Vector4(const Uint32 x, const Uint32 y, const Uint32 z = 0u, const Uint32 w = 0u)
    : v(_mm_castsi128_ps(_mm_set_epi32(w, z, y, x)))
{}

Vector4::Vector4(const float* src)
    : v(_mm_loadu_ps(src))
{}

Vector4::Vector4(const Float2& src)
{
    __m128 vx = _mm_load_ss(&src.x);
    __m128 vy = _mm_load_ss(&src.y);
    v = _mm_unpacklo_ps(vx, vy);
}

Vector4::Vector4(const Float3& src)
{
    __m128 vx = _mm_load_ss(&src.x);
    __m128 vy = _mm_load_ss(&src.y);
    __m128 vz = _mm_load_ss(&src.z);
    __m128 vxy = _mm_unpacklo_ps(vx, vy);
    v = _mm_movelh_ps(vxy, vz);
}

Vector4& Vector4::operator = (const Vector4& other)
{
    v = other.v;
    return *this;
}

const Vector4 Vector4::FromInteger(Int32 x)
{
    return _mm_cvtepi32_ps(_mm_set1_epi32(x));
}

const Vector4 Vector4::FromIntegers(Int32 x, Int32 y, Int32 z, Int32 w)
{
    return _mm_cvtepi32_ps(_mm_set_epi32(w, z, y, x));
}

const Vector4 Vector4::FromHalves(const Half* src)
{
#ifdef RT_USE_FP16C
    const __m128i v = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(src));
    return _mm_cvtph_ps(v);
#else // RT_USE_FP16C
    return Vector4(
        ConvertHalfToFloat(src[0]),
        ConvertHalfToFloat(src[1]),
        ConvertHalfToFloat(src[2]),
        ConvertHalfToFloat(src[3]));
#endif // RT_USE_FP16C
}

// Load & store ===================================================================================

const Vector4 Vector4::Load_Float2_Unsafe(const Float2* src)
{
    return Vector4(reinterpret_cast<const float*>(src));
}

const Vector4 Vector4::Load_Float3_Unsafe(const Float3* src)
{
    return Vector4(reinterpret_cast<const float*>(src));
}

const Vector4 Vector4::Load_Float2_Unsafe(const Float2& src)
{
    return Vector4(reinterpret_cast<const float*>(&src));
}

const Vector4 Vector4::Load_Float3_Unsafe(const Float3& src)
{
    return Vector4(reinterpret_cast<const float*>(&src));
}

const Vector4 Vector4::Load_2xUint8_Norm(const Uint8* src)
{
    const Vector4 mask{ 0xFFu, 0xFF00u, 0u, 0u };
    const Vector4 scale{ 1.0f / 255.0f, 1.0f / (256.0f * 255.0f), 0.0f, 0.0f };
    const Vector4 unsignedOffset{ 0.0f, 0.0f, 0.0f, 32768.0f * 65536.0f };

    __m128 vTemp = _mm_load_ps1((const float*)src);
    vTemp = _mm_and_ps(vTemp, mask.v);
    vTemp = _mm_xor_ps(vTemp, VECTOR_MASK_SIGN_W);

    // convert to float
    vTemp = _mm_cvtepi32_ps(_mm_castps_si128(vTemp));
    vTemp = _mm_add_ps(vTemp, unsignedOffset);
    return _mm_mul_ps(vTemp, scale);
}

const Vector4 Vector4::Load_4xUint8(const Uint8* src)
{
    const Vector4 mask{ 0xFFu, 0xFF00u, 0xFF0000u, 0xFF000000u };
    const Vector4 scale{ 1.0f, 1.0f / 256.0f, 1.0f / 65536.0f, 1.0f / (65536.0f * 256.0f) };
    const Vector4 unsignedOffset{ 0.0f, 0.0f, 0.0f, 32768.0f * 65536.0f };

    __m128 vTemp = _mm_load_ps1((const float*)src);
    vTemp = _mm_and_ps(vTemp, mask.v);
    vTemp = _mm_xor_ps(vTemp, VECTOR_MASK_SIGN_W);

    // convert to float
    vTemp = _mm_cvtepi32_ps(_mm_castps_si128(vTemp));
    vTemp = _mm_add_ps(vTemp, unsignedOffset);
    return _mm_mul_ps(vTemp, scale);
}

const Vector4 Vector4::Load_2xUint16_Norm(const Uint16* src)
{
    const Vector4 maskX16Y16{ 0x0000FFFFu, 0xFFFF0000u, 0u, 0u };
    const Vector4 flipY{ 0u, 0x80000000u, 0u, 0u };
    const Vector4 fixUpY16{ 1.0f / 65535.0f, 1.0f / (65535.0f * 65536.0f), 0.0f, 0.0f };
    const Vector4 fixAddY16{ 0.0f, 32768.0f * 65536.0f, 0.0f, 0.0f };

    __m128 vTemp = _mm_load_ps1(reinterpret_cast<const float *>(src));
    // Mask x&0xFFFF, y&0xFFFF0000,z&0,w&0
    vTemp = _mm_and_ps(vTemp, maskX16Y16);
    // y needs to be sign flipped
    vTemp = _mm_xor_ps(vTemp, flipY);
    // Convert to floating point numbers
    vTemp = _mm_cvtepi32_ps(_mm_castps_si128(vTemp));
    // y + 0x8000 to undo the signed order.
    vTemp = _mm_add_ps(vTemp, fixAddY16);
    // Y is 65536 times too large
    vTemp = _mm_mul_ps(vTemp, fixUpY16);

    return vTemp;
}

const Vector4 Vector4::Load_4xUint16(const Uint16* src)
{
    const Vector4 maskX16Y16Z16W16{ 0x0000FFFFu, 0x0000FFFFu, 0xFFFF0000u, 0xFFFF0000u };
    const __m128i shufflePattern = _mm_set_epi8(3, 2, 15, 14, 3, 2, 5, 4, 3, 2, 11, 10, 3, 2, 1, 0);

    // XXYYZZWWXXYYZZWW
    __m128d vIntd = _mm_load1_pd(reinterpret_cast<const double *>(src));
    // XX--ZZ----YY--WW
    __m128 masked = _mm_and_ps(_mm_castpd_ps(vIntd), maskX16Y16Z16W16);
    // --WW--ZZ--YY--XX
    __m128i reordered = _mm_shuffle_epi8(_mm_castps_si128(masked), shufflePattern);

    return _mm_cvtepi32_ps(reordered);
}

const Vector4 Vector4::LoadBGR_UNorm(const Uint8* src)
{
    const Vector4 mask{ 0xFF0000u, 0xFF00u, 0xFFu, 0x0u };
    const Vector4 scale{ 1.0f / 65536.0f / 255.0f, 1.0f / 256.0f / 255.0f, 1.0f / 255.0f, 0.0f };

    __m128 vTemp = _mm_load_ps1((const float*)src);
    vTemp = _mm_and_ps(vTemp, mask.v);

    // convert to float
    vTemp = _mm_cvtepi32_ps(_mm_castps_si128(vTemp));
    return _mm_mul_ps(vTemp, scale);
}

Uint32 Vector4::ToBGR() const
{
    const Vector4 scaled = (*this) * VECTOR_255;

    // convert to int and clamp to range
    __m128i vInt = _mm_cvttps_epi32(scaled);
    vInt = _mm_max_epi32(vInt, _mm_setzero_si128());
    vInt = _mm_min_epi32(vInt, _mm_set1_epi32(255));

    // extract RGB components:
    // in: 000000BB  000000GG  000000RR
    // out:                    00RRGGBB
    //const __m128i b = _mm_srli_si128(vInt, 8);
    //const __m128i g = _mm_srli_si128(vInt, 3);
    //const __m128i r = _mm_slli_epi32(vInt, 2 * 8);
    //const __m128i result = _mm_or_si128(r, _mm_or_si128(g, b));

    const __m128i shuffleMask = _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 4, 8);
    const __m128i result = _mm_shuffle_epi8(vInt, shuffleMask);

    return _mm_extract_epi32(result, 0);
}

Float2 Vector4::ToFloat2() const
{
    return Float2{ x, y };
}

Float3 Vector4::ToFloat3() const
{
    return Float3{ x, y, z };
}

template<Uint32 flipX, Uint32 flipY, Uint32 flipZ, Uint32 flipW>
const Vector4 Vector4::ChangeSign() const
{
    if (!(flipX || flipY || flipZ || flipW))
    {
        // no operation
        return *this;
    }

    // generate bit negation mask
    const Vector4 mask{ flipX ? 0x80000000 : 0, flipY ? 0x80000000 : 0, flipZ ? 0x80000000 : 0, flipW ? 0x80000000 : 0 };

    // flip sign bits
    return _mm_xor_ps(v, mask);
}

template<Uint32 maskX, Uint32 maskY, Uint32 maskZ, Uint32 maskW>
RT_FORCE_INLINE const Vector4 Vector4::MakeMask()
{
    static_assert(!(maskX == 0 && maskY == 0 && maskZ == 0 && maskW == 0), "Useless mask");
    static_assert(!(maskX && maskY && maskZ && maskW), "Useless mask");

    // generate bit negation mask
    return Vector4{ maskX ? 0xFFFFFFFF : 0, maskY ? 0xFFFFFFFF : 0, maskZ ? 0xFFFFFFFF : 0, maskW ? 0xFFFFFFFF : 0 };
}

// Elements rearrangement =========================================================================

template<Uint32 ix, Uint32 iy, Uint32 iz, Uint32 iw>
const Vector4 Vector4::Swizzle() const
{
    static_assert(ix < 4, "Invalid X element index");
    static_assert(iy < 4, "Invalid Y element index");
    static_assert(iz < 4, "Invalid Z element index");
    static_assert(iw < 4, "Invalid W element index");

    if (ix == 0 && iy == 1 && iz == 2 && iw == 3)
    {
        return *this;
    }
    else if (ix == 0 && iy == 0 && iz == 1 && iw == 1)
    {
        return _mm_unpacklo_ps(v, v);
    }
    else if (ix == 2 && iy == 2 && iz == 3 && iw == 3)
    {
        return _mm_unpackhi_ps(v, v);
    }
    else if (ix == 0 && iy == 1 && iz == 0 && iw == 1)
    {
        return _mm_movelh_ps(v, v);
    }
    else if (ix == 2 && iy == 3 && iz == 2 && iw == 3)
    {
        return _mm_movehl_ps(v, v);
    }
    else if (ix == 0 && iy == 0 && iz == 2 && iw == 2)
    {
        return _mm_moveldup_ps(v);
    }
    else if (ix == 1 && iy == 1 && iz == 3 && iw == 3)
    {
        return _mm_movehdup_ps(v);
    }

    return _mm_shuffle_ps(v, v, _MM_SHUFFLE(iw, iz, iy, ix));
}

const Vector4 Vector4::Swizzle(Uint32 ix, Uint32 iy, Uint32 iz, Uint32 iw) const
{
#ifdef RT_USE_AVX
    return _mm_permutevar_ps(v, _mm_set_epi32(iw, iz, iy, ix));
#else
    return Vector4{ f[ix], f[iy], f[iz], f[iw] };
#endif
}

const Vector4 Vector4::SplatX() const
{
    return Swizzle<0, 0, 0, 0>();
}

const Vector4 Vector4::SplatY() const
{
    return Swizzle<1, 1, 1, 1>();
}

const Vector4 Vector4::SplatZ() const
{
    return Swizzle<2, 2, 2, 2>();
}

const Vector4 Vector4::SplatW() const
{
    return Swizzle<3, 3, 3, 3>();
}

const Vector4 Vector4::Select(const Vector4& a, const Vector4& b, const VectorBool4& sel)
{
    return _mm_blendv_ps(a, b, sel.v);
}

template<Uint32 selX, Uint32 selY, Uint32 selZ, Uint32 selW>
const Vector4 Vector4::Select(const Vector4& a, const Vector4& b)
{
    static_assert(selX <= 1, "Invalid X index");
    static_assert(selY <= 1, "Invalid Y index");
    static_assert(selZ <= 1, "Invalid Z index");
    static_assert(selW <= 1, "Invalid W index");

    return _mm_blend_ps(a, b, selX | (selY << 1) | (selZ << 2) | (selW << 3));
}

// Logical operations =============================================================================

const Vector4 Vector4::operator& (const Vector4& b) const
{
    return _mm_and_ps(v, b);
}

const Vector4 Vector4::operator| (const Vector4& b) const
{
    return _mm_or_ps(v, b);
}

const Vector4 Vector4::operator^ (const Vector4& b) const
{
    return _mm_xor_ps(v, b);
}

Vector4& Vector4::operator&= (const Vector4& b)
{
    v = _mm_and_ps(v, b);
    return *this;
}

Vector4& Vector4::operator|= (const Vector4& b)
{
    v = _mm_or_ps(v, b);
    return *this;
}

Vector4& Vector4::operator^= (const Vector4& b)
{
    v = _mm_xor_ps(v, b);
    return *this;
}

// Simple arithmetics =============================================================================

const Vector4 Vector4::operator- () const
{
    return Vector4::Zero() - (*this);
}

const Vector4 Vector4::operator+ (const Vector4& b) const
{
    return _mm_add_ps(v, b);
}

const Vector4 Vector4::operator- (const Vector4& b) const
{
    return _mm_sub_ps(v, b);
}

const Vector4 Vector4::operator* (const Vector4& b) const
{
    return _mm_mul_ps(v, b);
}

const Vector4 Vector4::operator/ (const Vector4& b) const
{
    return _mm_div_ps(v, b);
}

const Vector4 Vector4::operator* (float b) const
{
    return _mm_mul_ps(v, _mm_set1_ps(b));
}

const Vector4 Vector4::operator/ (float b) const
{
    return _mm_div_ps(v, _mm_set1_ps(b));
}

const Vector4 operator*(float a, const Vector4& b)
{
    return _mm_mul_ps(b, _mm_set1_ps(a));
}


Vector4& Vector4::operator+= (const Vector4& b)
{
    v = _mm_add_ps(v, b);
    return *this;
}

Vector4& Vector4::operator-= (const Vector4& b)
{
    v = _mm_sub_ps(v, b);
    return *this;
}

Vector4& Vector4::operator*= (const Vector4& b)
{
    v = _mm_mul_ps(v, b);
    return *this;
}

Vector4& Vector4::operator/= (const Vector4& b)
{
    v = _mm_div_ps(v, b);
    return *this;
}

Vector4& Vector4::operator*= (float b)
{
    v = _mm_mul_ps(v, _mm_set1_ps(b));
    return *this;
}

Vector4& Vector4::operator/= (float b)
{
    v = _mm_div_ps(v, _mm_set1_ps(b));
    return *this;
}

const Vector4 Vector4::Mod1(const Vector4& x)
{
    return x - Vector4::Floor(x);
}

const Vector4 Vector4::MulAndAdd(const Vector4& a, const Vector4& b, const Vector4& c)
{
#ifdef RT_USE_FMA
    return _mm_fmadd_ps(a, b, c);
#else
    return a * b + c;
#endif
}

const Vector4 Vector4::MulAndSub(const Vector4& a, const Vector4& b, const Vector4& c)
{
#ifdef RT_USE_FMA
    return _mm_fmsub_ps(a, b, c);
#else
    return a * b - c;
#endif
}

const Vector4 Vector4::NegMulAndAdd(const Vector4& a, const Vector4& b, const Vector4& c)
{
#ifdef RT_USE_FMA
    return _mm_fnmadd_ps(a, b, c);
#else
    return -(a * b) + c;
#endif
}

const Vector4 Vector4::NegMulAndSub(const Vector4& a, const Vector4& b, const Vector4& c)
{
#ifdef RT_USE_FMA
    return _mm_fnmsub_ps(a, b, c);
#else
    return -(a * b) - c;
#endif
}

const Vector4 Vector4::MulAndAdd(const Vector4& a, const float b, const Vector4& c)
{
    return MulAndAdd(a, Vector4(b), c);
}

const Vector4 Vector4::MulAndSub(const Vector4& a, const float b, const Vector4& c)
{
    return MulAndSub(a, Vector4(b), c);
}

const Vector4 Vector4::NegMulAndAdd(const Vector4& a, const float b, const Vector4& c)
{
    return NegMulAndAdd(a, Vector4(b), c);
}

const Vector4 Vector4::NegMulAndSub(const Vector4& a, const float b, const Vector4& c)
{
    return NegMulAndSub(a, Vector4(b), c);
}

const Vector4 Vector4::Floor(const Vector4& v)
{
    return _mm_floor_ps(v);
}

const Vector4 Vector4::Sqrt4(const Vector4& V)
{
    return _mm_sqrt_ps(V);
}

const Vector4 Vector4::Reciprocal(const Vector4& V)
{
    return _mm_div_ps(VECTOR_ONE, V);
}

const Vector4 Vector4::FastReciprocal(const Vector4& v)
{
    const __m128 rcp = _mm_rcp_ps(v);
    const __m128 rcpSqr = _mm_mul_ps(rcp, rcp);
    const __m128 rcp2 = _mm_add_ps(rcp, rcp);
    return NegMulAndAdd(rcpSqr, v, rcp2);
}

const Vector4 Vector4::Lerp(const Vector4& v1, const Vector4& v2, const Vector4& weight)
{
    return MulAndAdd(v2 - v1, weight, v1);
}

const Vector4 Vector4::Lerp(const Vector4& v1, const Vector4& v2, float weight)
{
    return MulAndAdd(v2 - v1, Vector4(weight), v1);
}

const Vector4 Vector4::Min(const Vector4& a, const Vector4& b)
{
    return _mm_min_ps(a, b);
}

const Vector4 Vector4::Max(const Vector4& a, const Vector4& b)
{
    return _mm_max_ps(a, b);
}

const Vector4 Vector4::Abs(const Vector4& v)
{
    return _mm_and_ps(v, VECTOR_MASK_ABS);
}

const Vector4 Vector4::Clamped(const Vector4& min, const Vector4& max) const
{
    return Min(max, Max(min, *this));
}

int Vector4::GetSignMask() const
{
    return _mm_movemask_ps(v);
}

const Vector4 Vector4::HorizontalMax() const
{
    __m128 temp;
    temp = _mm_max_ps(v, _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 3, 0, 1)));
    temp = _mm_max_ps(temp, _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(1, 0, 3, 2)));
    return temp;
}

const VectorBool4 Vector4::operator == (const Vector4& b) const
{
    return _mm_cmpeq_ps(v, b.v);
}

const VectorBool4 Vector4::operator < (const Vector4& b) const
{
    return _mm_cmplt_ps(v, b.v);
}

const VectorBool4 Vector4::operator <= (const Vector4& b) const
{
    return _mm_cmple_ps(v, b.v);
}

const VectorBool4 Vector4::operator > (const Vector4& b) const
{
    return _mm_cmpgt_ps(v, b.v);
}

const VectorBool4 Vector4::operator >= (const Vector4& b) const
{
    return _mm_cmpge_ps(v, b.v);
}

const VectorBool4 Vector4::operator != (const Vector4& b) const
{
    return _mm_cmpneq_ps(v, b.v);
}

const Vector4 Vector4::Dot2V(const Vector4& v1, const Vector4& v2)
{
    return _mm_dp_ps(v1, v2, 0x3F);
}

const Vector4 Vector4::Dot3V(const Vector4& v1, const Vector4& v2)
{
    return _mm_dp_ps(v1, v2, 0x7F);
}

const Vector4 Vector4::Dot4V(const Vector4& v1, const Vector4& v2)
{
    return _mm_dp_ps(v1, v2, 0xFF);
}

float Vector4::Dot2(const Vector4& v1, const Vector4& v2)
{
    return _mm_cvtss_f32(Dot2V(v1, v2));
}

float Vector4::Dot3(const Vector4& v1, const Vector4& v2)
{
    return _mm_cvtss_f32(Dot3V(v1, v2));
}

float Vector4::Dot4(const Vector4& v1, const Vector4& v2)
{
    return _mm_cvtss_f32(Dot4V(v1, v2));
}

const Vector4 Vector4::Cross3(const Vector4& v1, const Vector4& v2)
{
    __m128 vTemp1 = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(3, 0, 2, 1));
    __m128 vTemp2 = _mm_shuffle_ps(v2, v2, _MM_SHUFFLE(3, 1, 0, 2));
    __m128 vResult = _mm_mul_ps(vTemp1, vTemp2);
    vTemp1 = _mm_shuffle_ps(vTemp1, vTemp1, _MM_SHUFFLE(3, 0, 2, 1));
    vTemp2 = _mm_shuffle_ps(vTemp2, vTemp2, _MM_SHUFFLE(3, 1, 0, 2));
    return NegMulAndAdd(vTemp1, vTemp2, vResult);
}

float Vector4::SqrLength2() const
{
    return Dot2(*this, *this);
}

float Vector4::Length2() const
{
    return _mm_cvtss_f32(Length2V());
}

const Vector4 Vector4::Length2V() const
{
    return _mm_sqrt_ps(Dot2V(v, v));
}

float Vector4::Length3() const
{
    return _mm_cvtss_f32(Length3V());
}

float Vector4::SqrLength3() const
{
    return Dot3(*this, *this);
}

const Vector4 Vector4::Length3V() const
{
    return _mm_sqrt_ps(Dot3V(v, v));
}

Vector4& Vector4::Normalize3()
{
    const __m128 vDot = Dot3V(v, v);
    const __m128 vTemp = _mm_sqrt_ps(vDot);
    v = _mm_div_ps(v, vTemp);
    return *this;
}

Vector4& Vector4::FastNormalize3()
{
    const __m128 vDot = Dot3V(v, v);
    v = _mm_mul_ps(v, _mm_rsqrt_ps(vDot));
    return *this;
}

const Vector4 Vector4::Normalized3() const
{
    Vector4 result = *this;
    result.Normalize3();
    return result;
}

const Vector4 Vector4::InvNormalized(Vector4& outInvNormalized) const
{
    const Vector4 len = Length3V();
    const Vector4 temp = Vector4::Select<0, 0, 0, 1>(*this, len);
    const Vector4 invTemp = Vector4::Reciprocal(temp); // [1/x, 1/y, 1/y, 1/length]

    outInvNormalized = len * invTemp;
    return (*this) * invTemp.w;
}

const Vector4 Vector4::FastNormalized3() const
{
    Vector4 result = *this;
    result.FastNormalize3();
    return result;
}

float Vector4::Length4() const
{
    return _mm_cvtss_f32(Length4V());
}

const Vector4 Vector4::Length4V() const
{
    return _mm_sqrt_ps(Dot4V(v, v));
}

float Vector4::SqrLength4() const
{
    return Dot4(*this, *this);
}

Vector4& Vector4::Normalize4()
{
    const __m128 vDot = Dot4V(v, v);
    const __m128 vTemp = _mm_sqrt_ps(vDot);
    v = _mm_div_ps(v, vTemp);
    return *this;
}

const Vector4 Vector4::Normalized4() const
{
    Vector4 result = *this;
    result.Normalize4();
    return result;
}

const Vector4 Vector4::Reflect3(const Vector4& i, const Vector4& n)
{
    // return (i - 2.0f * Dot(i, n) * n);
    const Vector4 vDot = Dot3V(i, n);
    return NegMulAndAdd(vDot + vDot, n, i);
}

bool Vector4::AlmostEqual(const Vector4& v1, const Vector4& v2, float epsilon)
{
    return (Abs(v1 - v2) < Vector4(epsilon)).All();
}

const VectorBool4 Vector4::IsZero() const
{
    return *this == Vector4::Zero();
}

// Check if any component is NaN
const VectorBool4 Vector4::IsNaN() const
{
    // check if exponent is all ones
    const __m128i epxMask = _mm_set1_epi32(0x7F800000);
    const __m128i expCheck = _mm_cmpeq_epi32(_mm_and_si128(vi, epxMask), epxMask);
    // check if mantissa is not zero
    const __m128i mantissaMask = _mm_set1_epi32(0x007FFFFF);
    const __m128i mantissaCheck = _mm_cmpeq_epi32(_mm_and_si128(vi, mantissaMask), _mm_setzero_si128());

    return _mm_andnot_si128(mantissaCheck, expCheck);
}

const VectorBool4 Vector4::IsInfinite() const
{
    // Mask off the sign bit
    __m128 temp = _mm_and_ps(v, VECTOR_MASK_ABS);
    // Compare to infinity
    return _mm_cmpeq_ps(temp, VECTOR_INF);
}

bool Vector4::IsValid() const
{
    // check if exponent is all ones
    const __m128i epxMask = _mm_set1_epi32(0x7F800000);
    const __m128i expCheck = _mm_cmpeq_epi32(_mm_and_si128(vi, epxMask), epxMask);
    return _mm_test_all_zeros(expCheck, expCheck);
}

void Vector4::Transpose3(Vector4& a, Vector4& b, Vector4& c)
{
    const __m128 t0 = _mm_unpacklo_ps(a, b);
    const __m128 t1 = _mm_unpackhi_ps(a, b);
    a = _mm_movelh_ps(t0, c);
    b = _mm_shuffle_ps(t0, c, _MM_SHUFFLE(3, 1, 3, 2));
    c = _mm_shuffle_ps(t1, c, _MM_SHUFFLE(3, 2, 1, 0));
}

const Vector4 Vector4::Orthogonalize(const Vector4& v, const Vector4& reference)
{
    // Gram–Schmidt process
    return Vector4::NegMulAndAdd(Vector4::Dot3V(v, reference), reference, v);
}

const Vector4 BipolarToUnipolar(const Vector4& x)
{
    return Vector4::MulAndAdd(x, VECTOR_HALVES, VECTOR_HALVES);
}

const Vector4 UnipolarToBipolar(const Vector4& x)
{
    return Vector4::MulAndSub(x, 2.0f, VECTOR_ONE);
}

} // namespace math
} // namespace rt
