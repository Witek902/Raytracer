#pragma once

#include "../Common.h"
#include "../Config.h"


namespace rt {


struct LocalCounters
{
    Uint32 numRayBoxTests;
    Uint32 numPassedRayBoxTests;
    Uint32 numRayTriangleTests;
    Uint32 numPassedRayTriangleTests;

    LocalCounters()
        : numRayBoxTests(0)
        , numPassedRayBoxTests(0)
        , numRayTriangleTests(0)
        , numPassedRayTriangleTests(0)
    { }
};

struct RayTracingCounters
{
    Uint32 numPrimaryRays;
#ifdef RT_ENABLE_INTERSECTION_COUNTERS
    Uint32 numRayBoxTests;
    Uint32 numPassedRayBoxTests;
    Uint32 numRayTriangleTests;
    Uint32 numPassedRayTriangleTests;
#endif // RT_ENABLE_INTERSECTION_COUNTERS
};


} // namespace rt
