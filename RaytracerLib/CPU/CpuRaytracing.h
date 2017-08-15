#pragma once

#include "../Math/Random.h"
#include "../Scene.h"

namespace rt {


struct RayTracingCounters
{
    Uint32 numPrimaryRays;
    Uint32 numShadowRays;
    Uint32 numReflectionRays;
    Uint32 numTransparencyRays;
    Uint32 numDiffuseRays;
#ifdef RT_ENABLE_INTERSECTION_COUNTERS
    Uint32 numRayBoxTests;
    Uint32 numPassedRayBoxTests;
    Uint32 numRayTriangleTests;
    Uint32 numPassedRayTriangleTests;
#endif // RT_ENABLE_INTERSECTION_COUNTERS
};


/**
 * A structure with local (per-thread) data.
 * It's like a hub for all global params (read only) and local state (read write).
 */
struct RayTracingContext
{
    // TODO ray value (for ray fading)

    // global rendering parameters
    const RaytracingParams& params;

    // per-thread pseudo-random number generator
    math::Random& randomGenerator;

    // per-thread counters
    RayTracingCounters& counters;

    RayTracingContext(math::Random& randomGenerator, const RaytracingParams& params, RayTracingCounters& counters)
        : params(params)
        , randomGenerator(randomGenerator)
        , counters(counters)
    { }
};


} // namespace rt
