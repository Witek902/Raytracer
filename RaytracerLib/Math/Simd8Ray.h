#pragma once

#include "Vector3x8.h"
#include "Ray.h"
#include "Simd4Ray.h"

namespace rt {
namespace math {

/**
 * 4 rays (SIMD version).
 */
class RT_ALIGN(32) Ray_Simd8
{
public:
    Vector3x8 dir;
    Vector3x8 origin;
    Vector3x8 invDir; 

    Ray_Simd8() = default;
    Ray_Simd8(const Ray_Simd8&) = default;
    Ray_Simd8& operator = (const Ray_Simd8&) = default;

    // splat single ray
    RT_FORCE_INLINE explicit Ray_Simd8(const Ray& ray)
        : dir(ray.dir)
        , origin(ray.origin)
        , invDir(ray.invDir)
    {
    }

    // from two SIMD-4 rays
    RT_FORCE_INLINE explicit Ray_Simd8(const Ray_Simd4& rayLo, const Ray_Simd4& rayHi)
        : dir(rayLo.dir, rayHi.dir)
        , origin(rayLo.origin, rayHi.origin)
        , invDir(rayLo.invDir, rayHi.invDir)
    {
    }

    // build SIMD ray from 8 rays
    RT_FORCE_INLINE Ray_Simd8(const Ray& ray0, const Ray& ray1, const Ray& ray2, const Ray& ray3,
                              const Ray& ray4, const Ray& ray5, const Ray& ray6, const Ray& ray7)
        : dir(ray0.dir, ray1.dir, ray2.dir, ray3.dir, ray4.dir, ray5.dir, ray6.dir, ray7.dir)
        , origin(ray0.origin, ray1.origin, ray2.origin, ray3.origin, ray4.origin, ray5.origin, ray6.origin, ray7.origin)
        , invDir(ray0.invDir, ray1.invDir, ray2.invDir, ray3.invDir, ray4.invDir, ray5.invDir, ray6.invDir, ray7.invDir)
    {
    }

    RT_FORCE_INLINE Ray_Simd8(const Vector3x8& origin, const Vector3x8& dir)
        : dir(dir)
        , origin(origin)
    {
        invDir = Vector3x8::FastReciprocal(dir);
    }

    // return rays octant if all the rays are in the same on
    // otherwise, returns 0xFFFFFFFF
    RT_FORCE_INLINE Uint32 GetOctant() const
    {
        const Int32 countX = PopCount(dir.x.GetSignMask());
        const Int32 countY = PopCount(dir.y.GetSignMask());
        const Int32 countZ = PopCount(dir.z.GetSignMask());

        const Uint32 xPart[9] = { 0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, 1u << 0u };
        const Uint32 yPart[9] = { 0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, 1u << 1u };
        const Uint32 zPart[9] = { 0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, 1u << 2u };

        return xPart[countX] | yPart[countY] | zPart[countZ];
    }
};

} // namespace math
} // namespace rt
