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
    Vector4 origin;
    Vector4 dir;
    Vector4 invDir;
    Vector4 originDivDir;

    RT_FORCE_INLINE Ray() = default;

    RT_FORCE_INLINE Ray(const Vector4& origin, const Vector4& direction)
        : origin(origin)
    {
        dir = direction.InvNormalized(invDir);
        originDivDir = origin * invDir;
    }

    // same as constructor, but direction must be already normalized
    RT_FORCE_INLINE static const Ray BuildUnsafe(const Vector4& origin, const Vector4& direction)
    {
        Ray ray;
        ray.origin = origin;
        ray.dir = direction;
        ray.invDir = Vector4::Reciprocal(direction);
        ray.originDivDir = origin * ray.invDir;
        return ray;
    }

    RT_FORCE_INLINE const Vector4 GetAtDistance(const float t) const
    {
        return Vector4::MulAndAdd(dir, t, origin);
    }

    RT_FORCE_INLINE bool IsValid() const
    {
        return origin.IsValid() && dir.IsValid();
    }
};

class RayBoxSegment
{
public:
    float nearDist;
    float farDist;
};


} // namespace math
} // namespace rt
