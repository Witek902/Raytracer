#pragma once

#include "RayLib.h"
#include "Config.h"
#include "MeshInstance.h"
#include "LightInstance.h"
#include "Math/Vector.h"
#include "Math/Random.h"

#include <vector>


namespace rt {

class Bitmap;
class Camera;

namespace math {
class Ray;
} // namespace math

using LightID = Uint32;
using MeshInstanceID = Uint32;


struct RaytracingParams
{
    Uint32 maxRayDepth;
};


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

//////////////////////////////////////////////////////////////////////////

/**
 * A structure with local (per-thread) data.
 * It's like a hub for all global params (read only) and local state (read write).
 */
struct RayTracingContext
{
    // current ray depth
    Uint32 depth;

    // TODO ray value (for ray fading)

    // global rendering parameters
    const RaytracingParams& params;

    // per-thread pseudo-random number generator
    math::Random& randomGenerator;

    // per-thread counters
    RayTracingCounters& counters;

    RayTracingContext(math::Random& randomGenerator, const RaytracingParams& params, RayTracingCounters& counters)
        : depth(0)
        , randomGenerator(randomGenerator)
        , params(params)
        , counters(counters)
    { }
};

//////////////////////////////////////////////////////////////////////////

/**
 * Rendering scene.
 * Allows for placing objects (meshes, lights, etc.) and raytracing them.
 */
class RAYLIB_API Scene
{
public:
    // add a mesh to the scene
    MeshInstanceID CreateMeshInstance(const MeshInstance& data);

    // add a light to the scene
    LightID CreateLightInstance(const LightInstance& data);

    // Perform scene raytracing
    // Result will be stored in the given bitmap (render target)
    bool Raytrace(const Camera& camera, Bitmap& renderTarget, const RaytracingParams& params);

    // trace single (non-SIMD) ray
    RT_FORCE_NOINLINE math::Vector TraceRay_Single(const math::Ray& ray, RayTracingContext& context) const;

private:
    RT_FORCE_NOINLINE void Raytrace_Single(const Camera& camera, Bitmap& renderTarget, const RaytracingParams& params);

    std::vector<MeshInstance> mMeshInstances;
    std::vector<LightInstance> mLightInstances;

    // TODO move to thread
    math::Random mRandomGenerator;
};

} // namespace rt
