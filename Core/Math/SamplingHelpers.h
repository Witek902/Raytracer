#pragma once

#include "../RayLib.h"
#include "Vector4.h"
#include "Vector2x8.h"
#include "Vector3x8.h"

namespace rt {
namespace math {

// Collection of helper functions to sample point on various 2D and 3D shapes.
// All the functions accepts 'u' parameter which is meant to be obtained from a sampler.
class SamplingHelpers
{
public:
    // get barycentric triangle coordinates
    RAYLIB_API static const Vector4 GetTriangle(const Float2 u);

    // get point on a circle (radius = 1.0)
    RAYLIB_API static const Vector4 GetCircle(const Float2 u);
    RAYLIB_API static const Vector2x8 GetCircle_Simd8(const Vector2x8 u);

    // get point on a regular hexagon
    // Note: 3 sample components are required
    RAYLIB_API static const Vector4 GetHexagon(const Float3 u);
    RAYLIB_API static const Vector2x8 GetHexagon_Simd8(const Vector2x8 u1, const Vector8 u2);

    // TODO
    /*
    // get point on a regular polygon
    RAYLIB_API static const Vector4 GetRegularPolygon(const Uint32 n, const Vector4 u);
    RAYLIB_API static const Vector2x8 GetRegularPolygon_Simd8(const Uint32 n, const Vector2x8 u);
    */

    // get point on a sphere (radius = 1.0)
    RAYLIB_API static const Vector4 GetSphere(const Float2 u);

    // get point on a hemisphere (uniform sampling, Z+ oriented)
    RAYLIB_API static const Vector4 GetHemishpere(const Float2 u);

    // get point on a hemisphere with cosine distribution (0 at equator, 1 at pole)
    // typical usage is Lambertian BRDF sampling
    RAYLIB_API static const Vector4 GetHemishpereCos(const Float2 u);

    // get 2D point with normal (Gaussian) distribution
    RAYLIB_API static const Vector4 GetFloatNormal2(const Float2 u);
};


} // namespace math
} // namespace rt
