#pragma once

#include "RayPacket.h"
#include "../Math/VectorInt8.h"

#include <float.h>
#include <assert.h>


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
        , triangleId(UINT32_MAX)
        , objectId(UINT32_MAX)
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
        , triangleId(UINT32_MAX)
        , objectId(UINT32_MAX)
    {}

    // extract single hit point
    RT_FORCE_INLINE HitPoint Get(Uint32 i) const
    {
        assert(i < 8);

        HitPoint result;
        result.distance = distance[i];
        result.u = u[i];
        result.v = v[i];
        result.triangleId = triangleId.u[i];
        result.objectId = objectId.u[i];
        return result;
    }
};

using HitPoint_Packet = HitPoint_Simd8[MaxRayPacketSize / 8];

} // namespace rt
