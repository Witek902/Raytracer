#pragma once

#include "../RayLib.h"
#include "../Math/Float3.h"
#include "../Containers/DynArray.h"

namespace rt {

namespace math {
class Random;
} // math

class GenericSampler
{
public:
    GenericSampler();
    ~GenericSampler() = default;

    // move to next frame
    void ResetFrame(const DynArray<uint32>& sample, bool useBlueNoise);

    // move to next pixel
    void ResetPixel(const uint32 x, const uint32 y);

    // get next sample
    // NOTE: effectively goes to next sample dimension
    uint32 GetInt();

    RT_FORCE_INLINE float GetFloat()
    {
        return math::Min(0.999999940395f, static_cast<float>(GetInt()) / 4294967296.0f);
    }

    RT_FORCE_INLINE const math::Float2 GetFloat2()
    {
        return math::Float2{ GetFloat(), GetFloat() };
    }

    RT_FORCE_INLINE const math::Float3 GetFloat3()
    {
        return math::Float3{ GetFloat(), GetFloat(), GetFloat() };
    }

    math::Random* fallbackGenerator = nullptr;

private:

    uint32 mBlueNoisePixelX = 0;
    uint32 mBlueNoisePixelY = 0;
    uint32 mBlueNoiseTextureLayers = 0;

    uint32 mSalt = 0;
    uint32 mSamplesGenerated = 0;

    const uint16* mBlueNoiseTexture = nullptr;

    DynArray<uint32> mCurrentSample;
};


} // namespace rt
