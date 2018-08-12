#pragma once

#include "Vector3x8.h"
#include "Box.h"


namespace rt {
namespace math {


/**
 * Eight boxes (SIMD version).
 */
class RT_ALIGN(32) Box_Simd8
{
public:
    Vector3x8 min;
    Vector3x8 max;

    Box_Simd8() = default;
    Box_Simd8(const Box_Simd8&) = default;
    Box_Simd8& operator = (const Box_Simd8&) = default;

    // build SIMD box from 8 boxes
    RT_FORCE_INLINE Box_Simd8(const Box& box0, const Box& box1, const Box& box2, const Box& box3,
                              const Box& box4, const Box& box5, const Box& box6, const Box& box7)
        : min(box0.min, box1.min, box2.min, box3.min, box4.min, box5.min, box6.min, box7.min)
        , max(box0.max, box1.max, box2.max, box3.max, box4.max, box5.max, box6.max, box7.max)
    { }

    // build SIMD box from 2 boxes (distribute to high and low lanes)
    RT_FORCE_INLINE Box_Simd8(const Box& boxLo, const Box& boxHi)
        : min(boxLo.min, boxLo.min, boxLo.min, boxLo.min,  boxHi.min, boxHi.min, boxHi.min, boxHi.min)
        , max(boxLo.max, boxLo.max, boxLo.max, boxLo.max,  boxHi.max, boxHi.max, boxHi.max, boxHi.max)
    { }

    // splat single box
    RT_FORCE_INLINE Box_Simd8(const Box& box)
        : min(box.min)
        , max(box.max)
    { }

    // build SIMD box from 8 boxes
    RT_FORCE_INLINE explicit Box_Simd8(const Box* boxes)
        : min(boxes[0].min, boxes[1].min, boxes[2].min, boxes[3].min, boxes[4].min, boxes[5].min, boxes[6].min, boxes[7].min)
        , max(boxes[0].max, boxes[1].max, boxes[2].max, boxes[3].max, boxes[4].max, boxes[5].max, boxes[6].max, boxes[7].max)
    { }
};


} // namespace math
} // namespace rt
