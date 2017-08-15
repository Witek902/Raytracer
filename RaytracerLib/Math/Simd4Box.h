#pragma once

#include "Simd4Vector3.h"
#include "Box.h"

namespace rt {
namespace math {

/**
 * 4 boxes (SIMD version).
 */
class RT_ALIGN(16) Box_Simd4
{
public:
    Vector3_Simd4 min;
    Vector3_Simd4 max;

    Box_Simd4() = default;
    Box_Simd4(const Box_Simd4&) = default;
    Box_Simd4& operator = (const Box_Simd4&) = default;

    // splat single box
    RT_FORCE_INLINE Box_Simd4(const Box& box)
        // TODO splat version
        : min(box.min, box.min, box.min, box.min)
        , max(box.max, box.max, box.max, box.max)
    { }

    // build SIMD box from 4 boxes
    RT_FORCE_INLINE Box_Simd4(const Box& box0, const Box& box1, const Box& box2, const Box& box3)
        : min(box0.min, box1.min, box2.min, box3.min)
        , max(box0.max, box1.max, box2.max, box3.max)
    { }

    // build SIMD box from 4 boxes
    RT_FORCE_INLINE explicit Box_Simd4(const Box* boxes)
        : min(boxes[0].min, boxes[1].min, boxes[2].min, boxes[3].min)
        , max(boxes[0].max, boxes[1].max, boxes[2].max, boxes[3].max)
    { }
};

} // namespace math
} // namespace rt
