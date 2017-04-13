#include "PCH.h"
#include "Random.h"
#include "Math.h"


namespace rt {
namespace math {


Random::Random(Uint64 seed)
    : mSeed(seed)
{
    mSeedV = _mm_set_epi32(0x2f558471, 0x61cb8acc, 0xd6974cff, 0x241e5c86);
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

Vector Random::GetVector4()
{
    // Integer XOR-shift
    mSeedV = _mm_xor_si128(mSeedV, _mm_slli_epi32(mSeedV, 13));
    mSeedV = _mm_xor_si128(mSeedV, _mm_srli_epi32(mSeedV, 17));
    mSeedV = _mm_xor_si128(mSeedV, _mm_slli_epi32(mSeedV, 5));

    // setup float mask
    __m128i v = mSeedV;
    v = _mm_and_si128(v, _mm_set1_epi32(0x007fffff));
    v = _mm_or_si128(v, _mm_set1_epi32(0x3f800000));
    v = _mm_add_epi32(v, _mm_set1_epi32(1));

    // convert to float and go from [1, 2) to [0, 1)
    Vector result = _mm_castsi128_ps(v);
    result -= VECTOR_ONE;

    return result;
}

} // namespace Math
} // namespace NFE
