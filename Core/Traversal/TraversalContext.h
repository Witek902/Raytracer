#pragma once

#include "HitPoint.h"

namespace rt {

namespace math {
class Ray;
class Ray_Simd8;
}

struct RenderingContext;

struct SingleTraversalContext
{
    const math::Ray& ray;
    HitPoint& hitPoint;
    RenderingContext& context;
};

struct SimdTraversalContext
{
    const math::Ray_Simd8& ray;
    HitPoint_Simd8& hitPoint;
    RenderingContext& context;
};

struct PacketTraversalContext
{
    RayPacket& ray;
    RenderingContext& context;

    void StoreIntersection(RayGroup& rayGroup, const math::Vector8& t, const math::VectorBool8& mask, Uint32 objectID, Uint32 subObjectID = 0) const;
};

} // namespace rt
