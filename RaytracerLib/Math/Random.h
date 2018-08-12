#pragma once

#include "../RayLib.h"
#include "Vector4.h"
#include "Vector2x8.h"

namespace rt {
namespace math {

class RT_ALIGN(32) RAYLIB_API Random
{
public:
    Random();

    // initialize seeds with new values, very slow
    void Reset();

    // get true random number, very slow
    static Uint32 GetEntropy();

    Uint64 GetLong();
    Uint32 GetInt();

    // Generate random float with uniform distribution from range [0.0f, 1.0f)
    float GetFloat();

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

	// generate random vector of 8 elements from range [-1.0f, 1.0f)
	Vector8 GetVector8Bipolar();

    // get random point on a circle (uniform distribution)
    Vector4 GetCircle();
    Vector2x8 GetCircle_Simd8();

    // get random point on a regular hexagon (uniform distribution)
    Vector4 GetHexagon();

    // generate random vector on a hemisphere with cosine distribution (0 at equator, 1 at pole)
    // typical usage: Lambertian BRDF sampling
    Vector4 GetHemishpereCos();

private:
	RT_FORCE_INLINE __m128i GetIntVector4();
	RT_FORCE_INLINE __m256i GetIntVector8();

    __m256i mSeedSimd8[2];
    __m128i mSeedSimd4[2];
    Uint64 mSeed[2];
};


} // namespace Math
} // namespace NFE
