#pragma once

namespace rt {
namespace math {

// Constructors ===================================================================================

Vector4::Vector4()
{
    v = _mm_setzero_ps();
}

Vector4::Vector4(const Float s)
{
    v = _mm_set1_ps(s);
}

Vector4::Vector4(const Float x, const Float y, const Float z, const Float w)
{
    v = _mm_set_ps(w, z, y, x);
}

Vector4::Vector4(const Int32 x, const Int32 y, const Int32 z, const Int32 w)
{
    v = _mm_castsi128_ps(_mm_set_epi32(w, z, y, x));
}

Vector4::Vector4(const Uint32 x, const Uint32 y, const Uint32 z, const Uint32 w)
{
    v = _mm_castsi128_ps(_mm_set_epi32(w, z, y, x));
}

Vector4::Vector4(const Float* src)
{
    v = _mm_loadu_ps(src);
}

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

void Vector4::Set(Float scalar)
{
    v = _mm_set1_ps(scalar);
}

Vector4 Vector4::FromIntegers(Uint32 x, Uint32 y, Uint32 z, Uint32 w)
{
    return Vector4(_mm_cvtepi32_ps(_mm_set_epi32(w, z, y, x)));
}

// Load & store ===================================================================================

Vector4 Vector4::Load4(const Uint8* src)
{
    static const Vector4 mask = { 0xFFu, 0xFF00u, 0xFF0000u, 0xFF000000u };
    static const Vector4 LoadUByte4Mul = {1.0f, 1.0f / 256.0f, 1.0f / 65536.0f, 1.0f / (65536.0f * 256.0f)};
    static const Vector4 unsignedOffset = { 0.0f, 0.0f, 0.0f, 32768.0f * 65536.0f };

    __m128 vTemp = _mm_load_ps1((const Float*)src);
    vTemp = _mm_and_ps(vTemp, mask.v);
    vTemp = _mm_xor_ps(vTemp, VECTOR_MASK_SIGN_W);

    // convert to Float
	vTemp = _mm_cvtepi32_ps(_mm_castps_si128(vTemp));
    vTemp = _mm_add_ps(vTemp, unsignedOffset);
    return _mm_mul_ps(vTemp, LoadUByte4Mul);
}

Vector4 Vector4::LoadBGR_UNorm(const Uint8* src)
{
	static const Vector4 mask = { 0xFF0000u, 0xFF00u, 0xFFu, 0x0u };
	static const Vector4 LoadUByte4Mul = { 1.0f / 65536.0f / 255.0f, 1.0f / 256.0f / 255.0f, 1.0f / 255.0f, 0.0f };

	__m128 vTemp = _mm_load_ps1((const Float*)src);
	vTemp = _mm_and_ps(vTemp, mask.v);

	// convert to Float
	vTemp = _mm_cvtepi32_ps(_mm_castps_si128(vTemp));
	return _mm_mul_ps(vTemp, LoadUByte4Mul);
}

void Vector4::Store4_NonTemporal(Uint8* dest) const
{
    // Clamp to Uint8 range
    const Vector4 vResult = Clamp(*this, Vector4(), VECTOR_255);

    // Convert to int & extract components
    __m128i vResulti = _mm_cvttps_epi32(vResult);
    __m128i Yi = _mm_srli_si128(vResulti, 3);
    __m128i Zi = _mm_srli_si128(vResulti, 6);
    __m128i Wi = _mm_srli_si128(vResulti, 9);

    vResulti = _mm_or_si128(_mm_or_si128(Wi, Zi), _mm_or_si128(Yi, vResulti));

	_mm_stream_si32(reinterpret_cast<Int32*>(dest), _mm_extract_epi32(vResulti, 0));
}

void Vector4::Store(Float* dest) const
{
    _mm_store_ss(dest, v);
}

void Vector4::Store(Float2* dest) const
{
    __m128 vy = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
    _mm_store_ss(&dest->x, v);
    _mm_store_ss(&dest->y, vy);
}

void Vector4::Store(Float3* dest) const
{
    __m128 vy = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
    __m128 vz = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
    _mm_store_ss(&dest->x, v);
    _mm_store_ss(&dest->y, vy);
    _mm_store_ss(&dest->z, vz);
}

Float2 Vector4::ToFloat2() const
{
    return Float2{ x, y };
}

Float3 Vector4::ToFloat3() const
{
    return Float3{ x, y, z };
}

template<bool x, bool y, bool z, bool w>
Vector4 Vector4::ChangeSign() const
{
    if (!(x || y || z || w))
    {
        // no negation
        return *this;
    }

    // generate bit negation mask
    static const Vector4 mask = {x ? 0x80000000 : 0, y ? 0x80000000 : 0, z ? 0x80000000 : 0, w ? 0x80000000 : 0};

    // flip sign bits
    return _mm_xor_ps(v, mask);
}

// Elements rearrangement =========================================================================

template<Uint32 ix, Uint32 iy, Uint32 iz, Uint32 iw>
Vector4 Vector4::Swizzle() const
{
    static_assert(ix < 4, "Invalid X element index");
    static_assert(iy < 4, "Invalid Y element index");
    static_assert(iz < 4, "Invalid Z element index");
    static_assert(iw < 4, "Invalid W element index");

    return _mm_shuffle_ps(v, v, _MM_SHUFFLE(iw, iz, iy, ix));
}

Vector4 Vector4::SplatX() const
{
    return _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0));
}

Vector4 Vector4::SplatY() const
{
    return _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
}

Vector4 Vector4::SplatZ() const
{
    return _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
}

Vector4 Vector4::SplatW() const
{
    return _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3));
}

Vector4 Vector4::SelectBySign(const Vector4& a, const Vector4& b, const Vector4& sel)
{
    return _mm_blendv_ps(a, b, sel);
}

// Logical operations =============================================================================

Vector4 Vector4::operator& (const Vector4& b) const
{
    return _mm_and_ps(v, b);
}

Vector4 Vector4::operator| (const Vector4& b) const
{
    return _mm_or_ps(v, b);
}

Vector4 Vector4::operator^ (const Vector4& b) const
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

Vector4 Vector4::operator- () const
{
    return Vector4() - (*this);
}

Vector4 Vector4::operator+ (const Vector4& b) const
{
    return _mm_add_ps(v, b);
}

Vector4 Vector4::operator- (const Vector4& b) const
{
    return _mm_sub_ps(v, b);
}

Vector4 Vector4::operator* (const Vector4& b) const
{
    return _mm_mul_ps(v, b);
}

Vector4 Vector4::operator/ (const Vector4& b) const
{
    return _mm_div_ps(v, b);
}

Vector4 Vector4::operator* (Float b) const
{
    return _mm_mul_ps(v, _mm_set1_ps(b));
}

Vector4 Vector4::operator/ (Float b) const
{
    return _mm_div_ps(v, _mm_set1_ps(b));
}

Vector4 operator*(Float a, const Vector4& b)
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

Vector4& Vector4::operator*= (Float b)
{
    v = _mm_mul_ps(v, _mm_set1_ps(b));
    return *this;
}

Vector4& Vector4::operator/= (Float b)
{
    v = _mm_div_ps(v, _mm_set1_ps(b));
    return *this;
}

Vector4 Vector4::MulAndAdd(const Vector4& a, const Vector4& b, const Vector4& c)
{
#ifdef RT_USE_FMA
    return _mm_fmadd_ps(a, b, c);
#else
    return a * b + c;
#endif
}

Vector4 Vector4::MulAndSub(const Vector4& a, const Vector4& b, const Vector4& c)
{
#ifdef RT_USE_FMA
    return _mm_fmsub_ps(a, b, c);
#else
    return a * b - c;
#endif
}

Vector4 Vector4::NegMulAndAdd(const Vector4& a, const Vector4& b, const Vector4& c)
{
#ifdef RT_USE_FMA
    return _mm_fnmadd_ps(a, b, c);
#else
    return -(a * b) + c;
#endif
}

Vector4 Vector4::NegMulAndSub(const Vector4& a, const Vector4& b, const Vector4& c)
{
#ifdef RT_USE_FMA
    return _mm_fnmsub_ps(a, b, c);
#else
    return c - a * b;
#endif
}


Vector4 Vector4::Floor(const Vector4& V)
{
    Vector4 vResult = _mm_sub_ps(V, _mm_set1_ps(0.49999f));
    __m128i vInt = _mm_cvtps_epi32(vResult);
    vResult = _mm_cvtepi32_ps(vInt);
    return vResult;
}


Vector4 Vector4::Sqrt(const Vector4& V)
{
    return _mm_sqrt_ss(V);
}


Vector4 Vector4::Sqrt4(const Vector4& V)
{
    return _mm_sqrt_ps(V);
}

Vector4 Vector4::Reciprocal(const Vector4& V)
{
    return _mm_div_ps(VECTOR_ONE, V);
}

Vector4 Vector4::FastReciprocal(const Vector4& v)
{
    const __m128 rcp = _mm_rcp_ps(v);
    const __m128 rcpSqr = _mm_mul_ps(rcp, rcp);
    const __m128 rcp2 = _mm_add_ps(rcp, rcp);
    return _mm_fnmadd_ps(rcpSqr, v, rcp2);
}

Vector4 Vector4::Lerp(const Vector4& v1, const Vector4& v2, const Vector4& weight)
{
    const Vector4 diff = v2 - v1;
    return Vector4::MulAndAdd(diff, weight, v1);
}

Vector4 Vector4::Lerp(const Vector4& v1, const Vector4& v2, Float weight)
{
    const Vector4 diff = v2 - v1;
    return Vector4::MulAndAdd(diff, Vector4(weight), v1);
}

Vector4 Vector4::Min(const Vector4& a, const Vector4& b)
{
    return _mm_min_ps(a, b);
}

Vector4 Vector4::Max(const Vector4& a, const Vector4& b)
{
    return _mm_max_ps(a, b);
}

Vector4 Vector4::Abs(const Vector4& v)
{
    return _mm_and_ps(v, VECTOR_MASK_ABS);
}

Vector4 Vector4::Clamped(const Vector4& min, const Vector4& max) const
{
    return Min(max, Max(min, *this));
}

int Vector4::GetSignMask() const
{
    return _mm_movemask_ps(v);
}

Vector4 Vector4::HorizontalMin() const
{
    __m128 temp;
    temp = _mm_min_ps(v, _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 3, 0, 1)));
    temp = _mm_min_ps(temp, _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(1, 0, 3, 2)));
    return temp;
}

Vector4 Vector4::HorizontalMax() const
{
    __m128 temp;
    temp = _mm_max_ps(v, _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 3, 0, 1)));
    temp = _mm_max_ps(temp, _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(1, 0, 3, 2)));
    return temp;
}

// Comparison functions ===========================================================================

int Vector4::EqualMask(const Vector4& v1, const Vector4& v2)
{
    return _mm_movemask_ps(_mm_cmpeq_ps(v1, v2));
}

int Vector4::LessMask(const Vector4& v1, const Vector4& v2)
{
    return _mm_movemask_ps(_mm_cmplt_ps(v1, v2));
}

int Vector4::LessEqMask(const Vector4& v1, const Vector4& v2)
{
    return _mm_movemask_ps(_mm_cmple_ps(v1, v2));
}

int Vector4::GreaterMask(const Vector4& v1, const Vector4& v2)
{
    return _mm_movemask_ps(_mm_cmpgt_ps(v1, v2));
}

int Vector4::GreaterEqMask(const Vector4& v1, const Vector4& v2)
{
    return _mm_movemask_ps(_mm_cmpge_ps(v1, v2));
}

int Vector4::NotEqualMask(const Vector4& v1, const Vector4& v2)
{
    return _mm_movemask_ps(_mm_cmpneq_ps(v1, v2));
}

// 2D vector comparison functions =================================================================

bool Vector4::Equal2(const Vector4& v1, const Vector4& v2)
{
    return (_mm_movemask_ps(_mm_cmpeq_ps(v1, v2)) & 0x3) == 0x3;
}

bool Vector4::Less2(const Vector4& v1, const Vector4& v2)
{
    return (_mm_movemask_ps(_mm_cmplt_ps(v1, v2)) & 0x3) == 0x3;
}

bool Vector4::LessEq2(const Vector4& v1, const Vector4& v2)
{
    return (_mm_movemask_ps(_mm_cmple_ps(v1, v2)) & 0x3) == 0x3;
}

bool Vector4::Greater2(const Vector4& v1, const Vector4& v2)
{
    return (_mm_movemask_ps(_mm_cmpgt_ps(v1, v2)) & 0x3) == 0x3;
}

bool Vector4::GreaterEq2(const Vector4& v1, const Vector4& v2)
{
    return (_mm_movemask_ps(_mm_cmpge_ps(v1, v2)) & 0x3) == 0x3;
}

bool Vector4::NotEqual2(const Vector4& v1, const Vector4& v2)
{
    return (_mm_movemask_ps(_mm_cmpneq_ps(v1, v2)) & 0x3) == 0x3;
}

// 3D vector comparison functions =================================================================

bool Vector4::Equal3(const Vector4& v1, const Vector4& v2)
{
    return (_mm_movemask_ps(_mm_cmpeq_ps(v1, v2)) & 0x7) == 0x7;
}

bool Vector4::Less3(const Vector4& v1, const Vector4& v2)
{
    return (_mm_movemask_ps(_mm_cmplt_ps(v1, v2)) & 0x7) == 0x7;
}

bool Vector4::LessEq3(const Vector4& v1, const Vector4& v2)
{
    return (_mm_movemask_ps(_mm_cmple_ps(v1, v2)) & 0x7) == 0x7;
}

bool Vector4::Greater3(const Vector4& v1, const Vector4& v2)
{
    return (_mm_movemask_ps(_mm_cmpgt_ps(v1, v2)) & 0x7) == 0x7;
}

bool Vector4::GreaterEq3(const Vector4& v1, const Vector4& v2)
{
    return (_mm_movemask_ps(_mm_cmpge_ps(v1, v2)) & 0x7) == 0x7;
}

bool Vector4::NotEqual3(const Vector4& v1, const Vector4& v2)
{
    return (_mm_movemask_ps(_mm_cmpneq_ps(v1, v2)) & 0x7) == 0x7;
}

// 4D vector comparison functions =================================================================

bool Vector4::operator== (const Vector4& b) const
{
    return EqualMask(*this, b) == 0xF;
}

bool Vector4::operator< (const Vector4& b) const
{
    return LessMask(*this, b) == 0xF;
}

bool Vector4::operator<= (const Vector4& b) const
{
    return LessEqMask(*this, b) == 0xF;
}

bool Vector4::operator> (const Vector4& b) const
{
    return GreaterMask(*this, b) == 0xF;
}

bool Vector4::operator>= (const Vector4& b) const
{
    return GreaterEqMask(*this, b) == 0xF;
}

bool Vector4::operator!= (const Vector4& b) const
{
    return NotEqualMask(*this, b) == 0xF;
}

// Geometry functions =============================================================================

Vector4 Vector4::Dot2V(const Vector4& v1, const Vector4& v2)
{
    return _mm_dp_ps(v1, v2, 0x3F);
}

Vector4 Vector4::Dot3V(const Vector4& v1, const Vector4& v2)
{
    return _mm_dp_ps(v1, v2, 0x7F);
}

Vector4 Vector4::Dot4V(const Vector4& v1, const Vector4& v2)
{
    return _mm_dp_ps(v1, v2, 0xFF);
}

Float Vector4::Dot2(const Vector4& v1, const Vector4& v2)
{
    Float result;
    _mm_store_ss(&result, Dot2V(v1, v2));
    return result;
}

Float Vector4::Dot3(const Vector4& v1, const Vector4& v2)
{
    Float result;
    _mm_store_ss(&result, Dot3V(v1, v2));
    return result;
}

Float Vector4::Dot4(const Vector4& v1, const Vector4& v2)
{
    Float result;
    _mm_store_ss(&result, Dot4V(v1, v2));
    return result;
}

Vector4 Vector4::Cross3(const Vector4& V1, const Vector4& V2)
{
    __m128 vTemp1 = _mm_shuffle_ps(V1, V1, _MM_SHUFFLE(3, 0, 2, 1));
    __m128 vTemp2 = _mm_shuffle_ps(V2, V2, _MM_SHUFFLE(3, 1, 0, 2));
    __m128 vResult = _mm_mul_ps(vTemp1, vTemp2);
    vTemp1 = _mm_shuffle_ps(vTemp1, vTemp1, _MM_SHUFFLE(3, 0, 2, 1));
    vTemp2 = _mm_shuffle_ps(vTemp2, vTemp2, _MM_SHUFFLE(3, 1, 0, 2));
    vResult = NegMulAndAdd(vTemp1, vTemp2, vResult);
    return _mm_and_ps(vResult, VECTOR_MASK_XYZ);
}

Float Vector4::Length2() const
{
    Float result;
    _mm_store_ss(&result, Length2V());
    return result;
}

Vector4 Vector4::Length2V() const
{
    const __m128 vDot = Dot2V(v, v);
    return _mm_sqrt_ps(vDot);
}

Float Vector4::Length3() const
{
    Float result;
    _mm_store_ss(&result, Length3V());
    return result;
}

Vector4 Vector4::Length3V() const
{
    const __m128 vDot = Dot3V(v, v);
    return _mm_sqrt_ps(vDot);
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

Vector4 Vector4::Normalized3() const
{
    Vector4 result = *this;
    result.Normalize3();
    return result;
}

Vector4 Vector4::FastNormalized3() const
{
    Vector4 result = *this;
    result.FastNormalize3();
    return result;
}

Float Vector4::Length4() const
{
    const __m128 vDot = Dot4V(v, v);

    Float result;
    _mm_store_ss(&result, _mm_sqrt_ss(vDot));
    return result;
}

Vector4 Vector4::Length4V() const
{
    const __m128 vDot = Dot4V(v, v);
    return _mm_sqrt_ps(vDot);
}

Vector4& Vector4::Normalize4()
{
    const __m128 vDot = Dot4V(v, v);
    const __m128 vTemp = _mm_sqrt_ps(vDot);
    v = _mm_div_ps(v, vTemp);
    return *this;
}

Vector4 Vector4::Normalized4() const
{
    Vector4 result = *this;
    result.Normalize4();
    return result;
}

Vector4 Vector4::Reflect3(const Vector4& i, const Vector4& n)
{
    const __m128 vDot = Dot3V(i, n);
    __m128 vTemp = _mm_add_ps(vDot, vDot); // vTemp = 2 * vDot
    vTemp = _mm_mul_ps(vTemp, n);
    return _mm_sub_ps(i, vTemp);
}

Vector4 Vector4::PlaneFromPoints(const Vector4& p1, const Vector4& p2, const Vector4& p3)
{
    Vector4 V21 = p1 - p2;
    Vector4 V31 = p1 - p3;
    Vector4 n = Vector4::Cross3(V21, V31).Normalized3();
    Vector4 d = Vector4::Dot3V(n, p1);
    d = _mm_mul_ps(d, VECTOR_MINUS_ONE);
    n = _mm_and_ps(n, VECTOR_MASK_XYZ);
    d = _mm_and_ps(d, VECTOR_MASK_W);
    return _mm_or_ps(d, n);
}

Vector4 Vector4::PlaneFromNormalAndPoint(const Vector4& normal, const Vector4& p)
{
    Vector4 d = Vector4::Dot3V(normal, p);
    d = _mm_mul_ps(d, VECTOR_MINUS_ONE);
    Vector4 n = _mm_and_ps(normal, VECTOR_MASK_XYZ);
    d = _mm_and_ps(d, VECTOR_MASK_W);
    return _mm_or_ps(d, n);
}

bool Vector4::PlanePointSide(const Vector4& plane, const Vector4& point)
{
    Vector4 vTemp2 = _mm_and_ps(point, VECTOR_MASK_XYZ);
    vTemp2 = _mm_or_ps(vTemp2, VECTOR_W);
    Vector4 vTemp = _mm_mul_ps(plane, vTemp2);
    // copy X to the Z position and Y to the W position
    vTemp2 = _mm_shuffle_ps(vTemp2, vTemp, _MM_SHUFFLE(1, 0, 0, 0));
    // add Z = X+Z; W = Y+W
    vTemp2 = _mm_add_ps(vTemp2, vTemp);
    // copy W to the Z position
    vTemp = _mm_shuffle_ps(vTemp, vTemp2, _MM_SHUFFLE(0, 3, 0, 0));
    // add Z and W together
    vTemp = _mm_add_ps(vTemp, vTemp2);
    // return Z >= 0.0f
    int mask = _mm_movemask_ps(_mm_cmpge_ps(vTemp, _mm_setzero_ps()));
    return (mask & (1 << 2)) != 0;
}

} // namespace math
} // namespace rt
