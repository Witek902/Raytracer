#pragma once

#include "../Math/Ray.h"
#include "../Math/Simd8Ray.h"
#include "../Math/VectorInt8.h"
#include "../BVH/BVH.h"


namespace rt {

// maximum number of rays in ray stream
// TODO experiment with this value
static const uint32 MaxRayPacketSize = 4096;

struct RT_ALIGN(4) ImageLocationInfo
{
    uint16 x;
    uint16 y;

    ImageLocationInfo() = default;
    RT_FORCE_INLINE ImageLocationInfo(uint32 x, uint32 y)
        : x((uint16)x)
        , y((uint16)y)
    { }
};

struct RT_ALIGN(32) RayGroup
{
    math::Ray_Simd8 rays[2];
    math::Vector8 maxDistances;
    math::VectorInt8 rayOffsets;
};

// packet of coherent rays (8-SIMD version)
struct RT_ALIGN(32) RayPacket
{
    static constexpr uint32 RaysPerGroup = 8;
    static constexpr uint32 MaxNumGroups = MaxRayPacketSize / RaysPerGroup;

    RayGroup groups[MaxNumGroups];

    // rays influence on the image (e.g. 1.0 for primary rays)
    math::Vector3x8 rayWeights[MaxNumGroups];

    // corresponding image pixels
    ImageLocationInfo imageLocations[MaxRayPacketSize];

    // number of rays (not groups!)
    uint32 numRays;

    RT_FORCE_INLINE RayPacket()
        : numRays(0)
    { }

    RT_FORCE_INLINE uint32 GetNumGroups() const
    {
        return (numRays + RaysPerGroup - 1) / RaysPerGroup;
    }

    RT_FORCE_INLINE void PushRay(const math::Ray& ray, const math::Vector4& weight, const ImageLocationInfo& location)
    {
        RT_ASSERT(numRays < MaxRayPacketSize);

        const uint32 groupIndex = numRays / RaysPerGroup;
        const uint32 rayIndex = numRays % RaysPerGroup;

        RayGroup& group = groups[groupIndex];
        group.rays[0].dir.x[rayIndex] = ray.dir.x;
        group.rays[0].dir.y[rayIndex] = ray.dir.y;
        group.rays[0].dir.z[rayIndex] = ray.dir.z;
        group.rays[0].origin.x[rayIndex] = ray.origin.x;
        group.rays[0].origin.y[rayIndex] = ray.origin.y;
        group.rays[0].origin.z[rayIndex] = ray.origin.z;
        group.rays[0].invDir.x[rayIndex] = ray.invDir.x;
        group.rays[0].invDir.y[rayIndex] = ray.invDir.y;
        group.rays[0].invDir.z[rayIndex] = ray.invDir.z;
        group.maxDistances[rayIndex] = FLT_MAX;
        group.rayOffsets[rayIndex] = numRays;

        rayWeights[groupIndex].x[rayIndex] = weight.x;
        rayWeights[groupIndex].y[rayIndex] = weight.y;
        rayWeights[groupIndex].z[rayIndex] = weight.z;

        imageLocations[numRays] = location;

        numRays++;
    }

    // TODO use non-temporal stores?
    RT_FORCE_INLINE void PushRays(const math::Ray_Simd8& rays, const math::Vector3x8& weights, const ImageLocationInfo* locations)
    {
        RT_ASSERT((numRays < MaxRayPacketSize) && (numRays % RaysPerGroup == 0));

        RayGroup& group = groups[numRays / RaysPerGroup];
        group.rays[0] = rays;
        group.maxDistances = math::VECTOR8_MAX;
        group.rayOffsets = math::VectorInt8(numRays) + math::VectorInt8(0, 1, 2, 3, 4, 5, 6, 7);

        rayWeights[numRays / RaysPerGroup] = weights;

        // Note: this should be replaced with a single MOVUPS instruction
        memcpy(imageLocations + numRays, locations, sizeof(ImageLocationInfo) * 8);

        numRays += RaysPerGroup;
    }

    RT_FORCE_INLINE void Clear()
    {
        numRays = 0;
    }
};

/*
struct RT_ALIGN(64) RayStream
{
    float rayOriginX[MaxRayPacketSize];
    float rayOriginY[MaxRayPacketSize];
    float rayOriginZ[MaxRayPacketSize];

    float rayDirX[MaxRayPacketSize];
    float rayDirY[MaxRayPacketSize];
    float rayDirZ[MaxRayPacketSize];

    float distance[MaxRayPacketSize];
};

struct RT_ALIGN(64) RayStreamHitData
{
    float distance[MaxRayPacketSize];
    float u[MaxRayPacketSize];
    float v[MaxRayPacketSize];
    uint32 subObjectId[MaxRayPacketSize];
    uint32 objectId[MaxRayPacketSize];
};
*/

} // namespace rt
