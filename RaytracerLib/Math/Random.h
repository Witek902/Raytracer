#pragma once

#include "../RayLib.h"
#include "Vector.h"

namespace rt {
namespace math {

RT_ALIGN(16)
class RAYLIB_API Random
{
private:
    __m128i mSeedV;
    Uint64 mSeed;

    // XOR-shift algorithm
    void Shuffle()
    {
        mSeed ^= (mSeed << 21);
        mSeed ^= (mSeed >> 35);
        mSeed ^= (mSeed << 4);
    }

public:
    Random(Uint64 seed = 0xcc13ad01e0b8b067);

    Uint64 GetLong();
    Uint32 GetInt();

    //Generate random float with uniform distribution from range (0.0f, 1.0f]
    float GetFloat();
    double GetDouble();

    // Generate random float with uniform distribution from range [-1.0f, 1.0f)
    // faster than "GetFloat()*2.0f-1.0f"
    float GetFloatBipolar();

    // generate random vector of 4 elements from range (0.0f, 1.0f]
    // this is much faster that using GetFloat() 4 times
    Vector GetVector4();
};


} // namespace Math
} // namespace NFE
