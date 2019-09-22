#pragma once

#include "../Common.h"
#include "../Config.h"


namespace rt {


struct LocalCounters
{
#ifdef RT_ENABLE_INTERSECTION_COUNTERS
    uint32 numRayBoxTests;
    uint32 numPassedRayBoxTests;
    uint32 numRayTriangleTests;
    uint32 numPassedRayTriangleTests;
#endif // RT_ENABLE_INTERSECTION_COUNTERS

    RT_FORCE_INLINE LocalCounters()
    {
        Reset();
    }

    RT_FORCE_INLINE void Reset()
    {
#ifdef RT_ENABLE_INTERSECTION_COUNTERS
        numRayBoxTests = 0;
        numPassedRayBoxTests = 0;
        numRayTriangleTests = 0;
        numPassedRayTriangleTests = 0;
#endif // RT_ENABLE_INTERSECTION_COUNTERS
    }
};


struct RayTracingCounters
{
    uint64 numRays;
    uint64 numShadowRays;
    uint64 numShadowRaysHit;
    uint64 numPrimaryRays;

#ifdef RT_ENABLE_INTERSECTION_COUNTERS
    uint64 numRayBoxTests;
    uint64 numPassedRayBoxTests;
    uint64 numRayTriangleTests;
    uint64 numPassedRayTriangleTests;
#endif // RT_ENABLE_INTERSECTION_COUNTERS


    RT_FORCE_INLINE void Reset()
    {
        numRays = 0;
        numShadowRays = 0;
        numShadowRaysHit = 0;
        numPrimaryRays = 0;

#ifdef RT_ENABLE_INTERSECTION_COUNTERS
        numRayBoxTests = 0;
        numPassedRayBoxTests = 0;
        numRayTriangleTests = 0;
        numPassedRayTriangleTests = 0;
#endif // RT_ENABLE_INTERSECTION_COUNTERS
    }


    RT_FORCE_INLINE void Append(const LocalCounters& other)
    {
        RT_UNUSED(other);
#ifdef RT_ENABLE_INTERSECTION_COUNTERS
        numRayBoxTests += other.numRayBoxTests;
        numPassedRayBoxTests += other.numPassedRayBoxTests;
        numRayTriangleTests += other.numRayTriangleTests;
        numPassedRayTriangleTests += other.numPassedRayTriangleTests;
#endif // RT_ENABLE_INTERSECTION_COUNTERS
    }


    void Append(const RayTracingCounters& other)
    {
        numRays += other.numRays;
        numShadowRays += other.numShadowRays;
        numShadowRaysHit += other.numShadowRaysHit;
        numPrimaryRays += other.numPrimaryRays;

#ifdef RT_ENABLE_INTERSECTION_COUNTERS
        numRayBoxTests += other.numRayBoxTests;
        numPassedRayBoxTests += other.numPassedRayBoxTests;
        numRayTriangleTests += other.numRayTriangleTests;
        numPassedRayTriangleTests += other.numPassedRayTriangleTests;
#endif // RT_ENABLE_INTERSECTION_COUNTERS
    }
};


} // namespace rt
