#pragma once

#include "Math.h"
#include "Vector.h"


namespace rt {
namespace math {

/**
 * Single ray (non-SIMD).
 */
class RT_ALIGN(16) Ray
{
public:
    Vector dir; // TODO remove?
    Vector invDir;
    Vector origin;

    Ray() {}

    Ray(const Vector& direction, const Vector& origin)
        : origin(origin)
    {
        dir = direction.FastNormalized3();
        invDir = Vector::FastReciprocal(dir);
    }
};

/**
 * 4 rays (SIMD version).
 */
class RT_ALIGN(16) SimdRay4
{
public:
    // TODO remove?
    Vector dirX;
    Vector dirY;
    Vector dirZ;

    Vector invDirX;
    Vector invDirY;
    Vector invDirZ;

    Vector originX;
    Vector originY;
    Vector originZ;

    // TODO
};

class RayBoxSegment
{
public:
    Float nearDist;
    Float farDist;
};

class RT_ALIGN(16) SimdRayBoxSegment4
{
public:
    Vector nearDists;
    Vector farDists;
};


} // namespace math
} // namespace rt
