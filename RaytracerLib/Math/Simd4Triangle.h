#pragma once

#include "Simd4Vector3.h"
#include "Triangle.h"


namespace rt {
namespace math {

/**
 * 4 triangles (SIMD version).
 */
class RT_ALIGN(16) Triangle_Simd4
{
public:
    Vector3_Simd4 v0;
    Vector3_Simd4 v1;
    Vector3_Simd4 v2;

    Triangle_Simd4() = default;
    Triangle_Simd4(const Triangle_Simd4&) = default;
    Triangle_Simd4& operator = (const Triangle_Simd4&) = default;

    // build SIMD triangle from 4 triangles
    RT_FORCE_INLINE Triangle_Simd4(const Triangle& t0, const Triangle& t1, const Triangle& t2, const Triangle& t3)
        : v0(t0.v0, t1.v0, t2.v0, t3.v0)
        , v1(t0.v1, t1.v1, t2.v1, t3.v1)
        , v2(t0.v2, t1.v2, t2.v2, t3.v2)
    { }
};


} // namespace math
} // namespace rt
