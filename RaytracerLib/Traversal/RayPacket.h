#pragma once

#include "../Math/Ray.h"
#include "../Math/Simd4Ray.h"
#include "../Math/Simd8Ray.h"
#include "../BVH/BVH.h"


namespace rt {

// maximum number of rays in ray stream
// TODO experiment with this value
static const Uint32 MaxRayPacketSize = 4096;

struct ImageLocationInfo
{
    Uint16 x;
    Uint16 y;
};

struct RT_ALIGN(32) RayGroup
{
    math::Ray_Simd8 rays;
    math::Vector8 maxDistances;
    Uint32 rayOffsets[8];
};

// packet of coherent rays (8-SIMD version)
struct RT_ALIGN(32) RayPacket
{
    static constexpr Uint32 RaysPerGroup = 8;
    static constexpr Uint32 MaxNumGroups = MaxRayPacketSize / RaysPerGroup;

    RayGroup groups[MaxNumGroups];

    // rays influence on the image (e.g. 1.0 for primary rays)
    // TODO: 3 half-floats should be fine
    math::Vector3x8 weights[MaxNumGroups];

    // corresponding image pixels
    ImageLocationInfo imageLocations[MaxRayPacketSize];

    // number of rays (not groups!)
    Uint32 numRays;

    RT_FORCE_INLINE RayPacket()
        : numRays(0)
    { }

    RT_FORCE_INLINE Uint32 GetNumGroups() const
    {
        return (numRays + 7) / 8;
    }

    RT_FORCE_INLINE void PushRays(const math::Ray_Simd8& rays, const math::Vector3x8& weight, const ImageLocationInfo& location)
    {
        RT_ASSERT(numRays < MaxRayPacketSize);

        RayGroup& group = groups[numRays / RaysPerGroup];
        group.rays = rays;
        group.maxDistances = math::VECTOR8_MAX;

        weights[numRays / RaysPerGroup] = weight;

        for (Uint32 i = 0; i < RaysPerGroup; ++i)
        {
            const Uint32 rayOffset = numRays + i;
            group.rayOffsets[i] = rayOffset;
            imageLocations[rayOffset] = location;
        }

        numRays += 8;
    }

    RT_FORCE_INLINE void Clear()
    {
        numRays = 0;
    }
};

/*
struct RT_ALIGN(64) RayStream
{
    Float rayOriginX[MaxRayPacketSize];
    Float rayOriginY[MaxRayPacketSize];
    Float rayOriginZ[MaxRayPacketSize];

    Float rayDirX[MaxRayPacketSize];
    Float rayDirY[MaxRayPacketSize];
    Float rayDirZ[MaxRayPacketSize];

    Float distance[MaxRayPacketSize];
};

struct RT_ALIGN(64) RayStreamHitData
{
    Float distance[MaxRayPacketSize];
    Float u[MaxRayPacketSize];
    Float v[MaxRayPacketSize];
    Uint32 triangleId[MaxRayPacketSize];
    Uint32 objectId[MaxRayPacketSize];
};
*/

} // namespace rt
