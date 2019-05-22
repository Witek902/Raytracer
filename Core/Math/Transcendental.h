#pragma once

#include "../Common.h"
#include "../RayLib.h"

#include "Vector4.h"
#include "Vector8.h"

namespace rt {
namespace math {


/**
 * Accurate sine and cosine.
 * @note    This is faster than sinf/cosf
 * @note    Maximum absolute error: about 5.0e-07
 */
RAYLIB_API float Sin(float x);
RAYLIB_API float Cos(float x);
RAYLIB_API const Vector4 Sin(const Vector4& x);
RAYLIB_API const Vector4 Cos(const Vector4& x);
RAYLIB_API const Vector8 Sin(const Vector8& x);
RAYLIB_API const Vector8 Cos(const Vector8& x);

// Compute sine and cosine in one go
RT_FORCE_INLINE const Vector4 SinCos(const float x)
{
    const Vector4 offset(0.0f, RT_PI / 2.0f, 0.0f, 0.0f);
    return Sin(Vector4(x) + offset) & Vector4::MakeMask<1,1,0,0>();
}

// Maximum absolute error: about 7.0e-5
RAYLIB_API float FastACos(float x);

/**
 * Fast atan2f.
 * Max relative error ~ 3.6e-5
 */
float FastATan2(const float y, const float x);

/**
 * Fast exponent.
 * @note    This is much faster than expf
 * @note    Maximum relative error: about 0.2%
 */
RAYLIB_API float FastExp(float x);
RAYLIB_API const Vector4 FastExp(const Vector4& x);

/**
 * Accurate natural logarithm.
 * @note    Maximum relative error: about 0.01%
 */
RAYLIB_API float Log(float x);

/**
 * Fast natural logarithm.
 * @note    This is faster logf
 * @note    Maximum relative error: about 0.07%
 */
RAYLIB_API float FastLog(float x);
RAYLIB_API const Vector4 FastLog(const Vector4& x);

} // namespace math
} // namespace rt
