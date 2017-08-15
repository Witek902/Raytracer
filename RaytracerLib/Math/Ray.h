#pragma once

#include "Math.h"
#include "Vector4.h"


namespace rt {
namespace math {

/**
 * Single ray (non-SIMD).
 */
class RT_ALIGN(16) Ray
{
public:
    Vector4 dir; // TODO remove?
    Vector4 invDir;
    Vector4 origin;
    Vector4 originDivDir;

    Ray() {}

    Ray(const Vector4& origin, const Vector4& direction)
        : origin(origin)
    {
        dir = direction.FastNormalized3();
        invDir = Vector4::FastReciprocal(dir);
        originDivDir = origin * invDir;
    }
};

class RayBoxSegment
{
public:
    Float nearDist;
    Float farDist;
};


} // namespace math
} // namespace rt
