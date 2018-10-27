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
    Vector4 invDir;
    Vector4 origin;
    Vector4 dir;

    RT_FORCE_INLINE Ray() = default;

    RT_FORCE_INLINE Ray(const Vector4& origin, const Vector4& direction)
        : origin(origin)
    {
        dir = direction.Normalized3();
        invDir = Vector4::Reciprocal(dir);
    }

    RT_FORCE_INLINE const Vector4 GetAtDistance(const Float t) const
    {
        return Vector4::MulAndAdd(dir, t, origin);
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
