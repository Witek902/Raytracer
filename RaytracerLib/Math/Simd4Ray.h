#pragma once

#include "Simd4Vector3.h"
#include "Ray.h"

namespace rt {
namespace math {

/**
 * 4 rays (SIMD version).
 */
class RT_ALIGN(16) Ray_Simd4
{
public:
    Vector3_Simd4 origin;
    Vector3_Simd4 dir;
    Vector3_Simd4 invDir;

    Ray_Simd4() = default;
    Ray_Simd4(const Ray_Simd4&) = default;
    Ray_Simd4& operator = (const Ray_Simd4&) = default;

    // splat single ray
    RT_FORCE_INLINE explicit Ray_Simd4(const Ray& ray)
        : dir(ray.dir)
        , origin(ray.origin)
    {
        invDir = Vector3_Simd4::FastReciprocal(dir);
    }

    // build SIMD ray from 4 rays
    RT_FORCE_INLINE Ray_Simd4(const Ray& ray0, const Ray& ray1, const Ray& ray2, const Ray& ray3)
        : dir(ray0.dir, ray1.dir, ray2.dir, ray3.dir)
        , origin(ray0.origin, ray1.origin, ray2.origin, ray3.origin)
    {
        invDir = Vector3_Simd4::FastReciprocal(dir);
    }
};

} // namespace math
} // namespace rt