#pragma once

#include "Math.h"
#include "Vector.h"


namespace rt {
namespace math {


class RT_ALIGN(16) Triangle
{
public:
    Vector v0, v1, v2;

    RT_FORCE_INLINE Triangle()
        : v0(), v1(), v2()
    {}

    RT_FORCE_INLINE Triangle(const Vector& v0, const Vector& v1, const Vector& v2)
        : v0(v0), v1(v1), v2(v2)
    {}
};


} // namespace math
} // namespace rt
