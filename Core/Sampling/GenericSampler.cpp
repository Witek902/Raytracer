#include "PCH.h"
#include "GenericSampler.h"
#include "../Math/Random.h"

namespace rt {

using namespace math;

RT_FORCE_INLINE static Uint32 XorShift(Uint32 x)
{
    x ^= x << 13u;
    x ^= x >> 17u;
    x ^= x << 5u;
    return x;
}

GenericSampler::GenericSampler(Random& fallbackGenerator)
    : mFallbackGenerator(fallbackGenerator)
{}

GenericSampler::~GenericSampler() = default;

void GenericSampler::ResetFrame(const DynArray<float>& seed)
{
    mSeed = seed;
}

void GenericSampler::ResetPixel(const Uint32 salt)
{
    mSalt = Hash(salt);
    mSamplesGenerated = 0;
}

float GenericSampler::GetFloat()
{
    Uint32 currentSample = mSamplesGenerated++;
    if (currentSample < mSeed.Size())
    {
        Uint32 salt = mSalt;
        mSalt = XorShift(mSalt);

        float sample = mSeed[currentSample] + float(salt) / float(UINT32_MAX);
        if (sample >= 1.0f)
        {
            sample -= 1.0f;
        }
        return sample;
    }

    return mFallbackGenerator.GetFloat();
}

const Float2 GenericSampler::GetFloat2()
{
    return Float2{ GetFloat(), GetFloat() };
}

const Float3 GenericSampler::GetFloat3()
{
    /*
    Vector4 v;

    const Uint32 currentSample = mSamplesGenerated;
    mSamplesGenerated += 3;

    if (currentSample < mSeed.Size())
    {
        const Uint32 salt0 = mSalt & INT32_MAX;
        mSalt = XorShift(mSalt);
        const Uint32 salt1 = mSalt & INT32_MAX;
        mSalt = XorShift(mSalt);
        const Uint32 salt2 = mSalt & INT32_MAX;
        mSalt = XorShift(mSalt);

        v = Vector4(Float3(mSeed.Data() + currentSample));
        v += Vector4::FromIntegers(salt0, salt1, salt2, 0) * (1.0f / float(INT32_MAX));

        // wrap
        v = Vector4::Select(v, v - VECTOR_ONE, v >= VECTOR_ONE);
    }
    else
    {
        v = mFallbackGenerator.GetVector4();
    }

    return v.ToFloat3();
    */

    return Float3{ GetFloat(), GetFloat(), GetFloat() };
}

} // namespace rt