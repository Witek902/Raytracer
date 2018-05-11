#pragma once

#include "../RayLib.h"
#include "Vector4.h"
#include "Simd8Vector2.h"

namespace rt {
namespace math {

class RT_ALIGN(32) RAYLIB_API Random
{
private:

    // Vector4 PRNG state
    __m256i mSeed0_Simd8;
    __m256i mSeed1_Simd8;

    // Vector4 PRNG state
    __m128i mSeed0;
    __m128i mSeed1;

    // Uint64
    Uint64 mSeed;

public:
    Random();

    // initialize seeds with new values, very slow
    void Reset();

    // get true random number, very slow
    static Uint64 GetEntropy();

    Uint64 GetLong();
    Uint32 GetInt();

    //Generate random float with uniform distribution from range [0.0f, 1.0f)
    float GetFloat();
    double GetDouble();

    // Generate random float with uniform distribution from range [-1.0f, 1.0f)
    // faster than "GetFloat()*2.0f-1.0f"
    float GetFloatBipolar();

    // generate random vector of 4 elements from range [0.0f, 1.0f)
    // this is much faster that using GetFloat() 4 times
    Vector4 GetVector4();

    // generate random vector of 4 elements from range [-1.0f, 1.0f)
    Vector4 GetVector4Bipolar();

    // generate random vector of 8 elements from range [0.0f, 1.0f)
    // this is much faster that using GetFloat() 8 times
    Vector8 GetVector8();

    // get random point on a circle (uniform distribution)
    Vector4 GetCircle();
    Vector2_Simd8 GetCircle_Simd8();

    // generate random vector on a hemisphere with cosine distribution (0 at equator, 1 at pole)
    // typical usage: Lambertian BRDF sampling
    Vector4 GetHemishpereCos();
};


} // namespace Math
} // namespace NFE
