#pragma once

#include "../RayLib.h"
#include "../Math/Random.h"

#include <vector>

namespace rt {

// multidimensional Halton sequence generator
// taken from: https://github.com/llxu2017/halton
class HaltonSequence
{
public:
    static constexpr size_t MaxDimensions = 4096;
    static constexpr size_t Width = 64;

    RAYLIB_API HaltonSequence();
    RAYLIB_API ~HaltonSequence();
    RAYLIB_API void Initialize(size_t mDimensions);

    RT_FORCE_INLINE size_t GetNumDimensions() const { return mDimensions; }

    RAYLIB_API void NextSample();
    RT_FORCE_INLINE double GetValue(size_t dimension) { return rnd[dimension][0]; }

private:
    Uint64 Permute(size_t i, Uint8 j);

    void ClearPermutation();
    void InitPrimes();
    void InitStart();
    void InitPowerBuffer();
    void InitExpansion();
    void InitPermutation();

    size_t mDimensions;
    std::vector<Uint64> mStarts;
    std::vector<Uint32> mBase;
    std::vector<std::vector<double>> rnd;
    std::vector<std::vector<Uint64>> digit;
    std::vector<std::vector<Uint64>> mPowerBuffer;
    Uint64 **ppm;
    math::Random mRandom;
};

} // namespace rt
