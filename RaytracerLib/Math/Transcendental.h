#pragma once

#include "../Common.h"

#include "Vector4.h"

namespace rt {
namespace math {


// TODO atan2
// TODO fast sin/cos


/**
 * Accurate sine and cosine.
 * @note    This is faster than sinf/cosf
 * @note    Maximum absolute error: about 5.0e-07
 */
float Sin(float x);
float Cos(float x);

Vector4 Sin(Vector4 x);

/**
 * Accurate inverse trigonometric functions.
 * @note    This is faster than asinf/acosf/atanf
 * @note    Maximum absolute error: about 2.0e-07
 */
float ASin(float x);
float ACos(float x);
float ATan(float x);

/**
 * Fast exponent.
 * @note    This is much faster than expf
 * @note    Maximum relative error: about 0.2%
 */
float FastExp(float x);

/**
 * Accurate natural logarithm.
 * @note    Maximum relative error: about 0.01%
 */
float Log(float x);

/**
 * Fast natural logarithm.
 * @note    This is faster logf
 * @note    Maximum relative error: about 0.07%
 */
float FastLog(float x);


} // namespace math
} // namespace rt
