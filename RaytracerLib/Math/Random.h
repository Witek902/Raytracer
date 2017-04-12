#pragma once

#include "../RayLib.h"


namespace rt {
namespace math {


class RAYLIB_API Random
{
private:
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

    /*
    // generate uniformly distributed float vectors
    Float2 GetFloat2();
    Float3 GetFloat3();
    Float4 GetFloat4();

    // Generate random float (vector) with Gaussian distribution. (SLOW)
    float GetFloatNormal();
    Float2 GetFloatNormal2();
    Float3 GetFloatNormal3();
    Float4 GetFloatNormal4();
    */
};


} // namespace Math
} // namespace NFE
