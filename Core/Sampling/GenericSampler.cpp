#include "PCH.h"
#include "GenericSampler.h"
#include "../Math/Random.h"

namespace rt {

using namespace math;

GenericSampler::GenericSampler(Random& fallbackGenerator)
    : mFallbackGenerator(fallbackGenerator)
    , mDimension(0)
{}

GenericSampler::~GenericSampler() = default;

void GenericSampler::ResetFrame(const std::vector<float>& seed)
{
    RT_ASSERT(seed.size() < MaxDimension);
    mDimension = static_cast<Uint32>(seed.size());
    memcpy(mSeed, seed.data(), seed.size() * sizeof(float));
}

void GenericSampler::ResetPixel(const Uint32 salt)
{
    mSalt = Hash(salt);
    mSamplesGenerated = 0;
}

float GenericSampler::GetFloat()
{
    Uint32 currentSample = mSamplesGenerated++;
    if (currentSample < mDimension)
    {
        // xorshift
        Uint32 salt = mSalt;
        mSalt ^= mSalt << 13u;
        mSalt ^= mSalt >> 17u;
        mSalt ^= mSalt << 5u;

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
    return Float2(GetFloat(), GetFloat());
}

const Float3 GenericSampler::GetFloat3()
{
    return Float3(GetFloat(), GetFloat(), GetFloat());
}

} // namespace rt