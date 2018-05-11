#include "PCH.h"
#include "Random.h"
#include "Math.h"
#include "Transcendental.h"

namespace rt {
namespace math {


Random::Random()
{
    Reset();
}

void Random::Reset()
{
    mSeed = GetEntropy();
    mSeed0 = _mm_set_epi64x(GetEntropy(), GetEntropy());
    mSeed1 = _mm_set_epi64x(GetEntropy(), GetEntropy());
    mSeed0_Simd8 = _mm256_set_epi64x(GetEntropy(), GetEntropy(), GetEntropy(), GetEntropy());
    mSeed1_Simd8 = _mm256_set_epi64x(GetEntropy(), GetEntropy(), GetEntropy(), GetEntropy());
}

Uint64 Random::GetEntropy()
{
    Uint64 val = 0;
    while (_rdrand64_step(&val) == 0) { }
    return val;
}

Uint64 Random::GetLong()
{
    Uint64 val = mSeed;

    // XOR-shift algorithm
    mSeed ^= (mSeed << 21);
    mSeed ^= (mSeed >> 35);
    mSeed ^= (mSeed << 4);

    return val;
}

Uint32 Random::GetInt()
{
    return static_cast<Uint32>(GetLong());
}

float Random::GetFloat()
{
    Bits32 myrand;
    myrand.ui = 1 + ((GetInt() & 0x007fffff) | 0x3f800000);
    return myrand.f - 1.0f;
}

float Random::GetFloatBipolar()
{
    Bits32 myrand;
    myrand.ui = 1 + ((GetInt() & 0x007fffff) | 0x40000000);
    return myrand.f - 3.0f;
}

double Random::GetDouble()
{
    Bits64 myrand;
    myrand.ui = 1L + ((GetLong() & 0x000fffffffffffffUL) | 0x3ff0000000000000UL);
    return myrand.f - 1.0;
}

Vector4 Random::GetVector4()
{
    // xorshift128+ algorithm
    const __m128i s0 = mSeed1;
    __m128i s1 = mSeed0;
    __m128i v = _mm_add_epi64(s0, s1);
    s1 = _mm_slli_epi64(s1, 23);
    const __m128i t0 = _mm_srli_epi64(s0, 5);
    const __m128i t1 = _mm_srli_epi64(s1, 18);
    mSeed0 = s0;
    mSeed1 = _mm_xor_si128(_mm_xor_si128(s1, t1), _mm_xor_si128(s0, t0));

    // setup float mask
    v = _mm_and_si128(v, _mm_set1_epi32(0x007fffffu));
    v = _mm_or_si128(v, _mm_set1_epi32(0x3f800000u));
    v = _mm_add_epi32(v, _mm_set1_epi32(1u));

    // convert to float and go from [1, 2) to [0, 1) range
    Vector4 result = _mm_castsi128_ps(v);
    result -= VECTOR_ONE;

    return result;
}

Vector4 Random::GetVector4Bipolar()
{
    // xorshift128+ algorithm
    const __m128i s0 = mSeed1;
    __m128i s1 = mSeed0;
    __m128i v = _mm_add_epi64(s0, s1);
    s1 = _mm_slli_epi64(s1, 23);
    const __m128i t0 = _mm_srli_epi64(s0, 5);
    const __m128i t1 = _mm_srli_epi64(s1, 18);
    mSeed0 = s0;
    mSeed1 = _mm_xor_si128(_mm_xor_si128(s1, t1), _mm_xor_si128(s0, t0));

    // setup float mask
    v = _mm_and_si128(v, _mm_set1_epi32(0x007fffffu));
    v = _mm_or_si128(v, _mm_set1_epi32(0x40000000u));
    v = _mm_add_epi32(v, _mm_set1_epi32(1u));

    // convert to float and go from [2, 4) to [-1, 1) range
    Vector4 result = _mm_castsi128_ps(v);
    result -= Vector4::Splat(3.0f);

    return result;
}

Vector8 Random::GetVector8()
{
    // xorshift128+ algorithm
    const __m256i s0 = mSeed1_Simd8;
    __m256i s1 = mSeed0_Simd8;
    __m256i v = _mm256_add_epi64(s0, s1);
    s1 = _mm256_slli_epi64(s1, 23);
    const __m256i t0 = _mm256_srli_epi64(s0, 5);
    const __m256i t1 = _mm256_srli_epi64(s1, 18);
    mSeed0_Simd8 = s0;
    mSeed1_Simd8 = _mm256_xor_si256(_mm256_xor_si256(s1, t1), _mm256_xor_si256(s0, t0));

    // setup float mask
    v = _mm256_and_si256(v, _mm256_set1_epi32(0x007fffffu));
    v = _mm256_or_si256(v, _mm256_set1_epi32(0x3f800000u));
    v = _mm256_add_epi32(v, _mm256_set1_epi32(1u));

    // convert to float and go from [1, 2) to [0, 1) range
    Vector8 result = _mm256_castsi256_ps(v);
    result -= VECTOR8_ONE;

    return result;
}

Vector4 Random::GetCircle()
{
    const Vector4 v = GetVector4();

    // angle (uniform distribution)
    const float theta = 2.0f * RT_PI * v[0];

    // radius (corrected distribution)
    const float u = v[1] + v[2];
    const float r = (u > 1.0f) ? (2.0f - u) : u;

    const Vector4 sinCos = Sin(Vector4(theta, theta + RT_PI / 2.0f));

    return r * sinCos;
}

Vector2_Simd8 Random::GetCircle_Simd8()
{
    // angle (uniform distribution)
    const Vector8 theta = (2.0f * RT_PI) * GetVector8();

    // radius (corrected distribution)
    const Vector8 u = GetVector8() + GetVector8();
    const Vector8 r = Vector8::SelectBySign(Vector8::Splat(2.0f) - u, u, u - Vector8::Splat(1.0f));

    const Vector8 vSin = Sin(theta);
    const Vector8 vCos = Sin(theta + Vector8::Splat(RT_PI / 2.0f));

    return { r * vSin, r * vCos };
}

Vector4 Random::GetHemishpereCos()
{
    const Vector4 u = GetVector4();

    // TODO optimize sqrtf, sin and cos (use approximations)
    const Vector4 t = Vector4::Sqrt4(Vector4(u[0], 1.0f - u[0]));

    float theta = 2.0f * RT_PI * u[1];
    return Vector4(t[0] * Sin(theta), t[0] * Cos(theta), t[1]);

}

} // namespace Math
} // namespace NFE
