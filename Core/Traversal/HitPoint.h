#pragma once

#include "RayPacket.h"
#include "../Math/VectorInt8.h"

#include <float.h>

constexpr uint32 RT_INVALID_OBJECT = UINT32_MAX;
constexpr uint32 RT_LIGHT_OBJECT = 0xFFFFFFFE;

namespace rt {

// Ray-scene intersection data (non-SIMD)
struct HitPoint
{
    union
    {
        struct
        {
            uint32 objectId;
            uint32 subObjectId;
        };

        uint64 combinedObjectId;
    };

    float distance;
    float u;
    float v;

    static constexpr float DefaultDistance = std::numeric_limits<float>::infinity();

    RT_FORCE_INLINE HitPoint()
        : objectId(RT_INVALID_OBJECT)
        , distance(DefaultDistance)
    {}

    RT_FORCE_INLINE void Set(float newDistance, uint32 newObjectId, uint32 newSubObjectId)
    {
        distance = newDistance;
        objectId = newObjectId;
        subObjectId = newSubObjectId;
    }

    // optimization: perform single 64-bit write instead of two 32-bit writes
    RT_FORCE_INLINE void Set(float newDistance, uint32 newObjectId)
    {
        distance = newDistance;
        combinedObjectId = newObjectId;
    }
};

// Ray-scene intersection data (SIMD-8)
struct RT_ALIGN(32) HitPoint_Simd8
{
    math::Vector8 distance;
    math::Vector8 u;
    math::Vector8 v;
    math::VectorInt8 objectId;
    math::VectorInt8 subObjectId;

    RT_FORCE_INLINE HitPoint_Simd8()
        : distance(math::VECTOR8_MAX)
        , objectId(RT_INVALID_OBJECT)
    {}

    // extract single hit point
    RT_FORCE_INLINE HitPoint Get(uint32 i) const
    {
        RT_ASSERT(i < 8);

        HitPoint result;
        result.distance = distance[i];
        result.u = u[i];
        result.v = v[i];
        result.objectId = objectId[i];
        result.subObjectId = subObjectId[i];
        return result;
    }
};

} // namespace rt
