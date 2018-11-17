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
#ifdef RT_USE_AVX2
        mSeedSimd8[i] = _mm256_set_epi32(GetEntropy(), GetEntropy(), GetEntropy(), GetEntropy(), GetEntropy(), GetEntropy(), GetEntropy(), GetEntropy());
#endif // RT_USE_AVX2
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
    return static_cast<Float>(GetInt()) / static_cast<Float>(std::numeric_limits<Uint32>::max());
}

const Float2 Random::GetFloat2()
{
    return GetVector4().ToFloat2();
}

const Float3 Random::GetFloat3()
{
    return GetVector4().ToFloat3();
}

float Random::GetFloatBipolar()
{
    Bits32 myrand;
    myrand.ui = (GetInt() & 0x007fffff) | 0x40000000;
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

const Vector4 Random::GetVector4()
{
    __m128i v = GetIntVector4();

    // setup float mask
    v = _mm_and_si128(v, _mm_set1_epi32(0x007fffffu));
    v = _mm_or_si128(v, _mm_set1_epi32(0x3f800000u));

    // convert to float and go from [1, 2) to [0, 1) range
    Vector4 result = _mm_castsi128_ps(v);
    result -= VECTOR_ONE;

    return result;
}

const Vector4 Random::GetVector4Bipolar()
{
    __m128i v = GetIntVector4();

    // setup float mask
    v = _mm_and_si128(v, _mm_set1_epi32(0x007fffffu));
    v = _mm_or_si128(v, _mm_set1_epi32(0x40000000u));

    // convert to float and go from [2, 4) to [-1, 1) range
    Vector4 result = _mm_castsi128_ps(v);
    result -= Vector4(3.0f);

    return result;
}

#ifdef RT_USE_AVX2

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

#else

__m256i Random::GetIntVector8()
{
    const __m128i v0 = GetIntVector4();
    const __m128i v1 = GetIntVector4();
    return _mm256_insertf128_si256(_mm256_castsi128_si256(v1), (v0), 1u);
}

#endif // RT_USE_AVX2

const Vector8 Random::GetVector8()
{
    VectorInt8 v = GetIntVector8();

    // setup float mask
    v &= VectorInt8(0x007fffffu);
    v |= VectorInt8(0x3f800000u);

    // convert to float and go from [1, 2) to [0, 1) range
    return v.CastToFloat() - VECTOR8_ONE;
}

const Vector8 Random::GetVector8Bipolar()
{
    VectorInt8 v = GetIntVector8();

    // setup float mask
    v &= VectorInt8(0x007fffffu);
    v |= VectorInt8(0x40000000u);

    // convert to float and go from [1, 2) to [0, 1) range
    return v.CastToFloat() - Vector8(3.0f);
}

const Vector2x8 Random::GetCircle_Simd8()
{
    // angle (uniform distribution)
    const Vector8 theta = (2.0f * RT_PI) * GetVector8();

    // radius (corrected distribution)
    const Vector8 r = Vector8::Sqrt(GetVector8());

    const Vector8 vSin = Sin(theta);
    const Vector8 vCos = Sin(theta + Vector8(RT_PI / 2.0f));

    return { r * vSin, r * vCos };
}

const Float2 Random::GetTriangle()
{
    const Float2 uv = GetFloat2();
    const Float u = sqrtf(uv.x);
    return { 1.0f - u, uv.y * u };
}

const Vector4 Random::GetCircle()
{
    const Vector4 v = GetVector4();

    // angle (uniform distribution)
    const float theta = 2.0f * RT_PI * v.x;

    // radius (corrected distribution)
    const float r = sqrtf(v.y);

    return r * SinCos(theta);
}

const Vector4 Random::GetHexagon()
{
    const Vector4 u = GetVector4();

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

    return Vector4(u.x * a.x + u.y * b.x, u.x * a.y + u.y * b.y, 0.0f, 0.0f);
}

const Vector4 Random::GetSphere()
{
    // based on http://mathworld.wolfram.com/SpherePointPicking.html

    const Vector4 u = GetVector4Bipolar();

    const Float t = sqrtf(1.0f - u.y * u.y);
    const Float theta = RT_PI * u.x;
    Vector4 result = t * SinCos(theta); // xy

    result.z = u.y;

    return result;
}

const Vector4 Random::GetHemishpere()
{
    Vector4 p = GetSphere();
    p.z = Abs(p.z);
    return p;
}

const Vector4 Random::GetHemishpereCos()
{
    const Vector4 u = GetVector4();

    const Float theta = 2.0f * RT_PI * u.y;
    const Float r = sqrtf(u.x); // this is required for the result vector to be normalized

    Vector4 result = r * SinCos(theta); // xy
    result.z = sqrtf(1.0f - u.x);

    return result;
}

const Vector4 Random::GetFloatNormal2()
{
    // Box-Muller method
    Vector4 uv = GetVector4();
    float temp = sqrtf(-2.0f * FastLog(uv.x));

    return temp * SinCos(2.0f * RT_PI * uv.y);
}

} // namespace Math
} // namespace NFE
