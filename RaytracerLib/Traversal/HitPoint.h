#pragma once

#include "RayPacket.h"
#include "../Math/VectorInt8.h"

#include <float.h>

constexpr Uint32 RT_INVALID_OBJECT = UINT32_MAX;
constexpr Uint32 RT_LIGHT_OBJECT = 0xFFFFFFFE;

namespace rt {

// Ray-scene intersection data (non-SIMD)
struct HitPoint
{
    float distance;
    float u;
    float v;
    Uint32 triangleId;
    Uint32 objectId;

    RT_FORCE_INLINE HitPoint()
        : distance(FLT_MAX)
        , u(0.0f)
        , v(0.0f)
        , triangleId(RT_INVALID_OBJECT)
        , objectId(RT_INVALID_OBJECT)
    {}
};

// Ray-scene intersection data (SIMD-8)
struct RT_ALIGN(32) HitPoint_Simd8
{
    math::Vector8 distance;
    math::Vector8 u;
    math::Vector8 v;
    math::VectorInt8 triangleId;
    math::VectorInt8 objectId;

    RT_FORCE_INLINE HitPoint_Simd8()
        : distance(math::VECTOR8_MAX)
        , triangleId(RT_INVALID_OBJECT)
        , objectId(RT_INVALID_OBJECT)
    {}

    // extract single hit point
    RT_FORCE_INLINE HitPoint Get(Uint32 i) const
    {
        RT_ASSERT(i < 8);

        HitPoint result;
        result.distance = distance[i];
        result.u = u[i];
        result.v = v[i];
        result.triangleId = triangleId[i];
        result.objectId = objectId[i];
        return result;
    }
};

using HitPoint_Packet = HitPoint_Simd8[MaxRayPacketSize / 8];

} // namespace rt
