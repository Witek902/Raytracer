#include "PCH.h"
#include "Random.h"
#include "Math.h"


namespace rt {
namespace math {


Random::Random()
{
    Reset(1);
}

void Random::Reset(Uint32 seed)
{
    seed = Hash(seed);

    mSeed = seed;
    mSeed0 = _mm_set_epi32(seed ^ 0x2f558471, seed ^ 0x61cb8acc, seed ^ 0xd6974cff, seed ^ 0x241e5c86);
    mSeed1 = _mm_set_epi32(seed ^ 0x24e64b29, seed ^ 0xe4f8e2d6, seed ^ 0xbb3399b9, seed ^ 0xa144f054);
}

Uint64 Random::GetLong()
{
    Shuffle();
    return mSeed;
}

Uint32 Random::GetInt()
{
    Shuffle();
    return static_cast<Uint32>(mSeed);
}


float Random::GetFloat()
{
    Shuffle();
    Bits32 myrand;
    myrand.ui = 1 + ((static_cast<Uint32>(mSeed) & 0x007fffff) | 0x3f800000);
    return myrand.f - 1.0f;
}

float Random::GetFloatBipolar()
{
    Shuffle();
    Bits32 myrand;
    myrand.ui = 1 + ((static_cast<Uint32>(mSeed) & 0x007fffff) | 0x40000000);
    return myrand.f - 3.0f;
}

double Random::GetDouble()
{
    Shuffle();
    Bits64 myrand;
    myrand.ui = 1L + ((mSeed & 0x000fffffffffffffUL) | 0x3ff0000000000000UL);
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

Vector4 Random::GetHemishpereCos()
{
    const Vector4 u = GetVector4();

    // TODO optimize sqrtf, sin and cos (use approximations)
    float r = sqrtf(u[0]);
    float theta = 2.0f * RT_PI * u[1];
    return Vector4(r * sinf(theta), r * cosf(theta), sqrtf(1.0f - u[0]));

}

} // namespace Math
} // namespace NFE
