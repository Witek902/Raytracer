#include "PCH.h"
#include "Random.h"
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
        mSeedSimd4[i] = VectorInt4(GetEntropy(), GetEntropy(), GetEntropy(), GetEntropy());
#ifdef RT_USE_AVX2
        mSeedSimd8[i] = VectorInt8(GetEntropy(), GetEntropy(), GetEntropy(), GetEntropy(), GetEntropy(), GetEntropy(), GetEntropy(), GetEntropy());
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
    return static_cast<float>(GetInt()) / static_cast<float>(std::numeric_limits<Uint32>::max());
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

VectorInt4 Random::GetIntVector4()
{
    // NOTE: xoroshiro128+ is faster when using general purpose registers, because there's
    // no rotate left/right instruction in SSE2 (it's only in AVX512)

    // xorshift128+ algorithm
    const VectorInt4 s0 = mSeedSimd4[1];
    VectorInt4 s1 = mSeedSimd4[0];
    VectorInt4 v = _mm_add_epi64(s0, s1);
    s1 = _mm_slli_epi64(s1, 23);
    const VectorInt4 t0 = _mm_srli_epi64(s0, 5);
    const VectorInt4 t1 = _mm_srli_epi64(s1, 18);
    mSeedSimd4[0] = s0;
    mSeedSimd4[1] = (s0 ^ s1) ^ (t0 ^ t1);
    return v;
}

const Vector4 Random::GetVector4()
{
    VectorInt4 v = GetIntVector4();

    // setup float mask
    v &= VectorInt4(0x007fffffu);
    v |= VectorInt4(0x3f800000u);

    // convert to float and go from [1, 2) to [0, 1) range
    return v.CastToFloat() - VECTOR_ONE;
}

const Vector4 Random::GetVector4Bipolar()
{
    VectorInt4 v = GetIntVector4();

    // setup float mask
    v &= VectorInt4(0x007fffffu);
    v |= VectorInt4(0x40000000u);

    // convert to float and go from [2, 4) to [-1, 1) range
    return v.CastToFloat() - Vector4(3.0f);
}

#ifdef RT_USE_AVX2

VectorInt8 Random::GetIntVector8()
{
    // NOTE: xoroshiro128+ is faster when using general purpose registers, because there's
    // no rotate left/right instruction in AVX2 (it's only in AVX512)

    // xorshift128+ algorithm
    const VectorInt8 s0 = mSeedSimd8[1];
    VectorInt8 s1 = mSeedSimd8[0];
    VectorInt8 v = _mm256_add_epi64(s0, s1);
    s1 = _mm256_slli_epi64(s1, 23);
    const VectorInt8 t0 = _mm256_srli_epi64(s0, 5);
    const VectorInt8 t1 = _mm256_srli_epi64(s1, 18);
    mSeedSimd8[0] = s0;
    mSeedSimd8[1] = (s0 ^ s1) ^ (t0 ^ t1);

    return v;
}

#else

VectorInt8 Random::GetIntVector8()
{
    return VectorInt8{ GetIntVector4(), GetIntVector4() };
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
    const float u = sqrtf(uv.x);
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

    constexpr Float2 hexVectors[] =
    {
        { -1.0f, 0.0f },
        { 0.5f, 0.8660254f }, // sqrt(3.0f) / 2.0f
        { 0.5f, -0.8660254f }, // sqrt(3.0f) / 2.0f
        { -1.0f, 0.0f },
    };

    const Uint32 x = GetInt() % 3u;
    const Float2 a = hexVectors[x];
    const Float2 b = hexVectors[x + 1];

    return Vector4(u.x * a.x + u.y * b.x, u.x * a.y + u.y * b.y, 0.0f, 0.0f);
}

const Vector2x8 Random::GetHexagon_Simd8()
{
    // TODO uint vector
    const VectorInt8 i = (GetIntVector8() & VectorInt8(0x7FFFFFFF)) % 3;
    const VectorInt8 j = i + 1;

    const Vector2x8 u{ GetVector8(), GetVector8() };

    const Vector8 hexVectorsX(-1.0f, 0.5f, 0.5f, -1.0f, -1.0f, 0.5f, 0.5f, -1.0f);
    const Vector8 hexVectorsY(0.0f, 0.8660254f, -0.8660254f, 0.0f, 0.0f, 0.8660254f, -0.8660254f, 0.0f);
    const Vector2x8 x{ _mm256_permutevar_ps(hexVectorsX, i), _mm256_permutevar_ps(hexVectorsX, j) };
    const Vector2x8 y{ _mm256_permutevar_ps(hexVectorsY, i), _mm256_permutevar_ps(hexVectorsY, j) };

    return { Vector2x8::Dot(u, x), Vector2x8::Dot(u, y) };
}

const Vector4 Random::GetRegularPolygon(const Uint32 n)
{
    RT_ASSERT(n >= 3, "Polygon must have at least 3 sides");

    // generate random point in a generic triangle
    const Float2 uv = GetVector4().ToFloat2();
    const float u = sqrtf(uv.x);
    const Float2 triangle(1.0f - u, uv.y * u);

    // base triangle size
    const float a = Sin(RT_PI / (float)n); // can be precomputed
    const float b = sqrtf(1.0f - a * a);

    // genrate point in base triangle
    const float sign = GetInt() % 2 ? 1.0f : -1.0f;
    const Vector4 base(b * (triangle.x + triangle.y), a * triangle.y * sign, 0.0f, 0.0f);

    // rotate
    const float alpha = RT_2PI * (float)(GetInt() % n) / (float)n;
    const Vector4 sinCosAlpha = SinCos(alpha);

    return Vector4(sinCosAlpha.y * base.x - sinCosAlpha.x * base.y, sinCosAlpha.y * base.y + sinCosAlpha.x * base.x, 0.0f, 0.0f);
}

const Vector2x8 Random::GetRegularPolygon_Simd8(const Uint32 n)
{
    RT_ASSERT(n >= 3, "Polygon must have at least 3 sides");

    const float invN = 1.0f / (float)n;

    // generate random point in a generic triangle
    const Vector2x8 uv{ GetVector8(), GetVector8() };
    const Vector8 u = Vector8::Sqrt(uv.x);
    const Vector2x8 triangle(Vector8(1.0f) - u, uv.y * u);

    // base triangle size
    const float a = Sin(RT_PI * invN); // can be precomputed
    const float b = sqrtf(1.0f - a * a);

    // genrate point in base triangle
    const float sign = GetInt() % 2 ? 1.0f : -1.0f;
    const Vector2x8 base(b * (triangle.x + triangle.y), a * triangle.y * sign);

    // rotate
    const VectorInt8 i = (GetIntVector8() & VectorInt8(0x7FFFFFFF)) % n;
    const Vector8 alpha = i.ConvertToFloat() * (RT_2PI * invN);
    const Vector8 sinAlpha = Sin(alpha);
    const Vector8 cosAlpha = Cos(alpha);

    return Vector2x8(cosAlpha * base.x - sinAlpha * base.y, cosAlpha * base.y + sinAlpha * base.x);
}

const Vector4 Random::GetSphere()
{
    // based on http://mathworld.wolfram.com/SpherePointPicking.html

    const Vector4 u = GetVector4Bipolar();

    const float t = sqrtf(1.0f - u.y * u.y);
    const float theta = RT_PI * u.x;
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

    const float theta = 2.0f * RT_PI * u.y;
    const float r = sqrtf(u.x); // this is required for the result vector to be normalized

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
