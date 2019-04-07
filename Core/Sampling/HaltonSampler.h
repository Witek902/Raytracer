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
    static constexpr Uint32 MaxDimensions = 4096;
    static constexpr Uint32 Width = 64;

    RAYLIB_API HaltonSequence();
    RAYLIB_API ~HaltonSequence();
    RAYLIB_API void Initialize(Uint32 mDimensions);

    RT_FORCE_INLINE Uint32 GetNumDimensions() const { return mDimensions; }

    RAYLIB_API void NextSample();
    RT_FORCE_INLINE double GetValue(Uint32 dimension) { return rnd[dimension][0]; }

private:
    Uint64 Permute(Uint32 i, Uint8 j);

    void ClearPermutation();
    void InitPrimes();
    void InitStart();
    void InitPowerBuffer();
    void InitExpansion();
    void InitPermutation();

    Uint32 mDimensions;
    DynArray<Uint64> mStarts;
    DynArray<Uint32> mBase;
    DynArray<DynArray<double>> rnd;
    DynArray<DynArray<Uint64>> digit;
    DynArray<DynArray<Uint64>> mPowerBuffer;
    Uint64 **ppm;
    math::Random mRandom;
};

} // namespace rt
