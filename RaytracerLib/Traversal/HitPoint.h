#pragma once

#include "RayPacket.h"

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

// Ray-scene intersection data (SIMD-4)
struct HitPoint_Simd4
{
    math::Vector4 distance;
    math::Vector4 u;
    math::Vector4 v;
    math::Vector4 triangleId;
    math::Vector4 objectId;

    RT_FORCE_INLINE HitPoint_Simd4()
        : distance(math::VECTOR_MAX)
        , triangleId(math::VECTOR_MASK_XYZW)
        , objectId(math::VECTOR_MASK_XYZW)
    {}

    // extract single hit point
    RT_FORCE_INLINE HitPoint Get(Uint32 i) const
    {
        assert(i < 4);

        HitPoint result;
        result.distance = distance[i];
        result.u = u[i];
        result.v = v[i];
        result.triangleId = triangleId.u[i];
        result.objectId = objectId.u[i];
        return result;
    }
};

// Ray-scene intersection data (SIMD-8)
struct HitPoint_Simd8
{
    math::Vector8 distance;
    math::Vector8 u;
    math::Vector8 v;
    math::Vector8 triangleId;
    math::Vector8 objectId;

    RT_FORCE_INLINE HitPoint_Simd8()
        : distance(math::VECTOR8_MAX)
        , triangleId(math::VECTOR8_MASK_ALL)
        , objectId(math::VECTOR8_MASK_ALL)
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

using HitPoint_Packet = HitPoint[MaxRayPacketSize];

} // namespace rt
