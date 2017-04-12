#pragma once

#include "Math.h"
#include "Vector.h"


namespace rt {
namespace math {

/**
 * Sphere
 */
class RT_ALIGN(16) Sphere
{
public:
    Vector origin; //< Sphere center
    float r; //< Sphere radius

    RT_FORCE_INLINE Sphere()
        : origin(), r()
    {}

    RT_FORCE_INLINE Sphere(const Vector& origin, float r)
        : origin(origin), r(r)
    {}

    RT_FORCE_INLINE float SupportVertex(const Vector& dir) const
    {
        Vector pos = origin + r * dir;
        return Vector::Dot3(dir, pos);
    }
};


} // namespace Math
} // namespace NFE
