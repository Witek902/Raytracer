#include "PCH.h"
#include "Random.h"
#include "Math.h"
#include "Transcendental.h"

namespace rt {
namespace math {

static RT_FORCE_INLINE Uint64 Rotl64(const Uint64 x, const int k)
{
    return (x << k) | (x >> (64 - k));
}

Random::Random()
{
    Reset();
}

void Random::Reset()
{
    for (Uint32 i = 0; i < 2; ++i)
    {
        mSeed[i] = ((Uint64)GetEntropy() << 32) | (Uint64)GetEntropy();
        mSeedSimd4[i] = _mm_set_epi32(GetEntropy(), GetEntropy(), GetEntropy(), GetEntropy());
        mSeedSimd8[i] = _mm256_set_epi32(GetEntropy(), GetEntropy(), GetEntropy(), GetEntropy(), GetEntropy(), GetEntropy(), GetEntropy(), GetEntropy());
    }
}

Uint32 Random::GetEntropy()
{
    Uint32 val = 0;
    while (_rdrand32_step(&val) == 0) {}
    return val;
}

Uint64 Random::GetLong()
{
    // xoroshiro128+ algorithm
    // http://xoshiro.di.unimi.it/xoroshiro128plus.c

    const Uint64 s0 = mSeed[0];
    Uint64 s1 = mSeed[1];
    const Uint64 result = s0 + s1;

    s1 ^= s0;
    mSeed[0] = Rotl64(s0, 24) ^ s1 ^ (s1 << 16);
    mSeed[1] = Rotl64(s1, 37);

    return result;
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

Float2 Random::GetFloat2()
{
    return GetVector4().ToFloat2();
}

Float3 Random::GetFloat3()
{
    return GetVector4().ToFloat3();
}

float Random::GetFloatBipolar()
{
    Bits32 myrand;
    myrand.ui = 1 + ((GetInt() & 0x007fffff) | 0x40000000);
    return myrand.f - 3.0f;
}

__m128i Random::GetIntVector4()
{
    // NOTE: xoroshiro128+ is faster when using general purpose registers, because there's
    // no rotate left/right instruction in SSE2 (it's only in AVX512)

    // xorshift128+ algorithm
    const __m128i s0 = mSeedSimd4[1];
    __m128i s1 = mSeedSimd4[0];
    __m128i v = _mm_add_epi64(s0, s1);
    s1 = _mm_slli_epi64(s1, 23);
    const __m128i t0 = _mm_srli_epi64(s0, 5);
    const __m128i t1 = _mm_srli_epi64(s1, 18);
    mSeedSimd4[0] = s0;
    mSeedSimd4[1] = _mm_xor_si128(_mm_xor_si128(s0, s1), _mm_xor_si128(t0, t1));
    return v;
}

__m256i Random::GetIntVector8()
{
    // NOTE: xoroshiro128+ is faster when using general purpose registers, because there's
    // no rotate left/right instruction in AVX2 (it's only in AVX512)

    // xorshift128+ algorithm
    const __m256i s0 = mSeedSimd8[1];
    __m256i s1 = mSeedSimd8[0];
    __m256i v = _mm256_add_epi64(s0, s1);
    s1 = _mm256_slli_epi64(s1, 23);
    const __m256i t0 = _mm256_srli_epi64(s0, 5);
    const __m256i t1 = _mm256_srli_epi64(s1, 18);
    mSeedSimd8[0] = s0;
    mSeedSimd8[1] = _mm256_xor_si256(_mm256_xor_si256(s0, s1), _mm256_xor_si256(t0, t1));
    return v;
}

Vector4 Random::GetVector4()
{
    __m128i v = GetIntVector4();

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
    __m128i v = GetIntVector4();

    // setup float mask
    v = _mm_and_si128(v, _mm_set1_epi32(0x007fffffu));
    v = _mm_or_si128(v, _mm_set1_epi32(0x40000000u));
    v = _mm_add_epi32(v, _mm_set1_epi32(1u));

    // convert to float and go from [2, 4) to [-1, 1) range
    Vector4 result = _mm_castsi128_ps(v);
    result -= Vector4(3.0f);

    return result;
}

Vector8 Random::GetVector8()
{
    __m256i v = GetIntVector8();

    // setup float mask
    v = _mm256_and_si256(v, _mm256_set1_epi32(0x007fffffu));
    v = _mm256_or_si256(v, _mm256_set1_epi32(0x3f800000u));
    v = _mm256_add_epi32(v, _mm256_set1_epi32(1u));

    // convert to float and go from [1, 2) to [0, 1) range
    Vector8 result = _mm256_castsi256_ps(v);
    result -= VECTOR8_ONE;

    return result;
}

Vector8 Random::GetVector8Bipolar()
{
    __m256i v = GetIntVector8();

    // setup float mask
    v = _mm256_and_si256(v, _mm256_set1_epi32(0x007fffffu));
    v = _mm256_or_si256(v, _mm256_set1_epi32(0x40000000u));
    v = _mm256_add_epi32(v, _mm256_set1_epi32(1u));

    // convert to float and go from [2, 4) to [-1, 1) range
    Vector8 result = _mm256_castsi256_ps(v);
    result -= Vector8(3.0f);

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

    const Vector4 sinCos = Sin(Vector4(theta, theta + RT_PI / 2.0f, 0.0f, 0.0f));

    return r * sinCos;
}

Vector2x8 Random::GetCircle_Simd8()
{
    // angle (uniform distribution)
    const Vector8 theta = (2.0f * RT_PI) * GetVector8();

    // radius (corrected distribution)
    const Vector8 u = GetVector8() + GetVector8();
    const Vector8 r = Vector8::SelectBySign(Vector8(2.0f) - u, u, u - Vector8(1.0f));

    const Vector8 vSin = Sin(theta);
    const Vector8 vCos = Sin(theta + Vector8(RT_PI / 2.0f));

    return { r * vSin, r * vCos };
}

Vector4 Random::GetHexagon()
{
    constexpr Float2 g_hexVectors[] =
    {
        { -1.0f, 0.0f },
        { 0.5f, 0.8660254f }, // sqrt(3.0f) / 2.0f
        { 0.5f, -0.8660254f }, // sqrt(3.0f) / 2.0f
        { -1.0f, 0.0f },
    };

    const Uint32 x = GetInt() % 3u;
    const Float2 a = g_hexVectors[x];
    const Float2 b = g_hexVectors[x + 1];
    const Vector4 xy = GetVector4();

    return Vector4(xy.x * a.x + xy.y * b.x, xy.x * a.y + xy.y * b.y, 0.0f, 0.0f);
}

Vector4 Random::GetHemishpereCos()
{
    const Vector4 u = GetVector4();

    // TODO optimize sqrtf, sin and cos (use approximations)
    const Vector4 t = Vector4::Sqrt4(Vector4(u[0], 1.0f - u[0], 0.0f, 0.0f));

    float theta = 2.0f * RT_PI * u[1];
    return Vector4(t[0] * Sin(theta), t[0] * Cos(theta), t[1], 0.0f);
}

Vector4 Random::GetFloatNormal2()
{
    Vector4 result;

    // Box-Muller method
    Vector4 uv = GetVector4();
    float temp = sqrtf(-2.0f * FastLog(uv.x));

    result.x = temp * Cos(2.0f * RT_PI * uv.y);
    result.y = temp * Sin(2.0f * RT_PI * uv.y);
    return result;
}

} // namespace Math
} // namespace NFE
