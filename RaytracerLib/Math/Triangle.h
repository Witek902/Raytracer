#pragma once

#include "Math.h"
#include "Vector4.h"


namespace rt {
namespace math {


class RT_ALIGN(16) Triangle
{
public:
    Vector4 v0, v1, v2;

    RT_FORCE_INLINE Triangle()
        : v0(), v1(), v2()
    {}

    RT_FORCE_INLINE Triangle(const Vector4& v0, const Vector4& v1, const Vector4& v2)
        : v0(v0), v1(v1), v2(v2)
    {}
};


} // namespace math
} // namespace rt
