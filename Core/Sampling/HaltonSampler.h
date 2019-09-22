#pragma once

#include "../RayLib.h"
#include "../Math/Random.h"
#include "../Containers/DynArray.h"

namespace rt {

// multidimensional Halton sequence generator
// taken from: https://github.com/llxu2017/halton
class HaltonSequence
{
public:
    static constexpr uint32 MaxDimensions = 4096;
    static constexpr uint32 Width = 64;

    RAYLIB_API HaltonSequence();
    RAYLIB_API ~HaltonSequence();
    RAYLIB_API void Initialize(uint32 mDimensions);

    RT_FORCE_INLINE uint32 GetNumDimensions() const { return mDimensions; }

    RAYLIB_API void NextSample();

    RT_FORCE_INLINE double GetDouble(uint32 dimension) { return rnd[dimension][0]; }
    RT_FORCE_INLINE uint32 GetInt(uint32 dimension) { return uint32(rnd[dimension][0] * (double)UINT32_MAX); }

private:
    uint64 Permute(uint32 i, uint8 j);

    void ClearPermutation();
    void InitPrimes();
    void InitStart();
    void InitPowerBuffer();
    void InitExpansion();
    void InitPermutation();

    uint32 mDimensions;
    DynArray<uint64> mStarts;
    DynArray<uint32> mBase;
    DynArray<DynArray<double>> rnd;
    DynArray<DynArray<uint64>> digit;
    DynArray<DynArray<uint64>> mPowerBuffer;
    uint64 **ppm;
    math::Random mRandom;
};

} // namespace rt
