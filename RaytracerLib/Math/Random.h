#pragma once

#include "../RayLib.h"
#include "Vector4.h"

namespace rt {
namespace math {

RT_ALIGN(16)
class RAYLIB_API Random
{
private:
    __m128i mSeed0;
    __m128i mSeed1;

    Uint64 mSeed;

    // XOR-shift algorithm
    void Shuffle()
    {
        mSeed ^= (mSeed << 21);
        mSeed ^= (mSeed >> 35);
        mSeed ^= (mSeed << 4);
    }

public:
    Random();

    void Reset(Uint32 seed);

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
    Vector4 GetVector4();

    // get random point on a circle (uniform distribution)
    Vector4 GetCircle();

    // generate random vector on a hemisphere with cosine distribution (0 at equator, 1 at pole)
    RT_FORCE_NOINLINE Vector4 GetHemishpereCos();
};


} // namespace Math
} // namespace NFE
