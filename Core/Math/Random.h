#pragma once

#include "../RayLib.h"
#include "Float2.h"
#include "Float3.h"
#include "Vector4.h"
#include "Vector2x8.h"
#include "VectorInt4.h"
#include "VectorInt8.h"

namespace rt {
namespace math {

// Pseudorandom number generator
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
    double GetDouble();
    const Float2 GetFloat2();
    const Float3 GetFloat3();

    // Generate random float with uniform distribution from range [-1.0f, 1.0f)
    // faster than "GetFloat()*2.0f-1.0f"
    float GetFloatBipolar();

    // generate random vector of 4 elements from range [0.0f, 1.0f)
    // this is much faster that using GetFloat() 4 times
    const Vector4 GetVector4();

    // generate random vector of 4 elements from range [-1.0f, 1.0f)
    const Vector4 GetVector4Bipolar();

    // generate random vector of 8 elements from range [0.0f, 1.0f)
    // this is much faster that using GetFloat() 8 times
    const Vector8 GetVector8();

    // generate random vector of 8 elements from range [-1.0f, 1.0f)
    const Vector8 GetVector8Bipolar();

private:
    RT_FORCE_INLINE VectorInt8 GetIntVector8();
    RT_FORCE_INLINE VectorInt4 GetIntVector4();

#ifdef RT_USE_AVX2
    VectorInt8 mSeedSimd8[2];
#endif // RT_USE_AVX2

    VectorInt4 mSeedSimd4[2];

    Uint64 mSeed[2];
};


} // namespace math
} // namespace rt
