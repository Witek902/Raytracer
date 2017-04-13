#pragma once

#include "RayLib.h"
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
    Uint32 numShadowRays;
    Uint32 numReflectionRays;
    Uint32 numTransparencyRays;
    Uint32 numDiffuseRays;
    // TODO ray-tri intersections
    // TODO ray-box intersections
};

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

    RT_FORCE_INLINE RayTracingContext(math::Random& randomGenerator, const RaytracingParams& params, RayTracingCounters& counters)
        : depth(0)
        , randomGenerator(randomGenerator)
        , params(params)
        , counters(counters)
    { }
};


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
    RT_FORCE_NOINLINE math::Vector TraceRaySingle(const math::Ray& ray, RayTracingContext& context);

private:
    RT_FORCE_NOINLINE void RaytraceSingle(const Camera& camera, Bitmap& renderTarget, const RaytracingParams& params);

    std::vector<MeshInstance> mMeshInstances;
    std::vector<LightInstance> mLightInstances;

    // TODO move to thread
    math::Random mRandomGenerator;
};

} // namespace rt
