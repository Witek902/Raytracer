#pragma once

namespace rt {

namespace math {
class Ray;
class Ray_Simd8;
}

struct HitPoint;
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
    HitPoint_Packet& hitPoint;
    RenderingContext& context;
};

} // namespace rt
