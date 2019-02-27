#pragma once

#include "Math.h"
#include "Vector4.h"


namespace rt {
namespace math {

/**
 * Sphere
 */
class RT_ALIGN(16) Sphere
{
public:
    Vector4 origin; //< Sphere center
    float r; //< Sphere radius

    RT_FORCE_INLINE Sphere()
        : origin(), r()
    {}

    RT_FORCE_INLINE Sphere(const Vector4& origin, float r)
        : origin(origin), r(r)
    {}

    RT_FORCE_INLINE float SupportVertex(const Vector4& dir) const
    {
        Vector4 pos = origin + r * dir;
        return Vector4::Dot3(dir, pos);
    }
};


} // namespace math
} // namespace rt