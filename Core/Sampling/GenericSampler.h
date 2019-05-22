#pragma once

#include "../RayLib.h"
#include "../Math/Vector4.h"
#include "../Containers/DynArray.h"

namespace rt {

namespace math {
class Random;
} // math

class GenericSampler
{
public:
    GenericSampler(math::Random& fallbackGenerator);
    ~GenericSampler();

    RT_FORCE_INLINE math::Random& GetFallbackGenerator() { return mFallbackGenerator; }

    void ResetFrame(const DynArray<float>& seed);

    void ResetPixel(const Uint32 x, const Uint32 y);

    // get next sample
    float GetFloat();
    const math::Float2 GetFloat2();
    const math::Float3 GetFloat3();

private:

    Uint32 mSalt;
    Uint32 mSamplesGenerated;

    DynArray<float> mSeed;

    math::Random& mFallbackGenerator;
};


} // namespace rt
