#pragma once

#include "Simd8Vector3.h"
#include "Triangle.h"


namespace rt {
namespace math {

/**
 * 8 triangles (SIMD version).
 */
class RT_ALIGN(32) Triangle_Simd8
{
public:
    Vector3_Simd8 v0;
    Vector3_Simd8 v1;
    Vector3_Simd8 v2;

    Triangle_Simd8() = default;
    Triangle_Simd8(const Triangle_Simd8&) = default;
    Triangle_Simd8& operator = (const Triangle_Simd8&) = default;

    // build SIMD triangle from 8 triangles
    RT_FORCE_INLINE Triangle_Simd8(const Triangle& t0, const Triangle& t1, const Triangle& t2, const Triangle& t3,
                                   const Triangle& t4, const Triangle& t5, const Triangle& t6, const Triangle& t7)
        : v0(t0.v0, t1.v0, t2.v0, t3.v0, t4.v0, t5.v0, t6.v0, t7.v0)
        , v1(t0.v1, t1.v1, t2.v1, t3.v1, t4.v1, t5.v1, t6.v1, t7.v1)
        , v2(t0.v2, t1.v2, t2.v2, t3.v2, t4.v2, t5.v2, t6.v2, t7.v2)
    { }
};


} // namespace math
} // namespace rt
