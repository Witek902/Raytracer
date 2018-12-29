#pragma once

#include "Vector3x8.h"
#include "Triangle.h"


namespace rt {
namespace math {

/**
 * 8 triangles (SIMD version).
 */
class RT_ALIGN(32) Triangle_Simd8
{
public:
    Vector3x8 v0;
    Vector3x8 edge1;
    Vector3x8 edge2;

    Triangle_Simd8() = default;
    Triangle_Simd8(const Triangle_Simd8&) = default;
    Triangle_Simd8& operator = (const Triangle_Simd8&) = default;

    // splat single triangle
    RT_FORCE_INLINE explicit Triangle_Simd8(const Triangle& tri)
        : v0(tri.v0)
        , edge1(tri.v1 - tri.v0)
        , edge2(tri.v2 - tri.v0)
    { }
};


} // namespace math
} // namespace rt
