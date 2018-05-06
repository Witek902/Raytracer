#pragma once

#include "Simd8Vector3.h"
#include "Ray.h"

namespace rt {
namespace math {

/**
 * 4 rays (SIMD version).
 */
class RT_ALIGN(32) Ray_Simd8
{
public:
    Vector3_Simd8 origin;
    Vector3_Simd8 dir;
    Vector3_Simd8 invDir;
    Vector3_Simd8 originDivDir;

    Ray_Simd8() = default;
    Ray_Simd8(const Ray_Simd8&) = default;
    Ray_Simd8& operator = (const Ray_Simd8&) = default;

    // splat single ray
    RT_FORCE_INLINE explicit Ray_Simd8(const Ray& ray)
        : dir(ray.dir)
        , origin(ray.origin)
        , invDir(ray.invDir)
    {
        originDivDir = origin * invDir;
    }

    // from two SIMD-4 rays
    RT_FORCE_INLINE explicit Ray_Simd8(const Ray_Simd4& rayLo, const Ray_Simd4& rayHi)
        : origin(rayLo.origin, rayHi.origin)
        , dir(rayLo.dir, rayHi.dir)
        , invDir(rayLo.invDir, rayHi.invDir)
    {
        originDivDir = origin * invDir;
    }

    // build SIMD ray from 8 rays
    RT_FORCE_INLINE Ray_Simd8(const Ray& ray0, const Ray& ray1, const Ray& ray2, const Ray& ray3,
                              const Ray& ray4, const Ray& ray5, const Ray& ray6, const Ray& ray7)
        : dir(ray0.dir, ray1.dir, ray2.dir, ray3.dir, ray4.dir, ray5.dir, ray6.dir, ray7.dir)
        , origin(ray0.origin, ray1.origin, ray2.origin, ray3.origin, ray4.origin, ray5.origin, ray6.origin, ray7.origin)
        , invDir(ray0.invDir, ray1.invDir, ray2.invDir, ray3.invDir, ray4.invDir, ray5.invDir, ray6.invDir, ray7.invDir)
    {
        originDivDir = origin * invDir;
    }
};

} // namespace math
} // namespace rt
