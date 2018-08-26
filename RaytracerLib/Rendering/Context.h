#pragma once

#include "Counters.h"

#include "../Traversal/RayPacket.h"
#include "../Traversal/HitPoint.h"

#include "../Utils/Bitmap.h" // TODO remove

#include "../Math/Random.h"

namespace rt {

enum class RenderingMode : Uint8
{
    Regular = 0,

    // material
    BaseColor,                  // visualize base color of the first intersection

    // geometry
    Depth,                      // visualize depth
    Position,                   // visualize world-space position
    Normals,                    // visualize normal vectors (in world space)
    Tangents,
    Bitangents,
    TexCoords,
    TriangleID,                 // draw every triangle with random color


#ifdef RT_ENABLE_INTERSECTION_COUNTERS
    // stats
    RayBoxIntersection,         // visualize number of performed ray-box intersections
    RayBoxIntersectionPassed,   // visualize number of passed ray-box intersections
    RayTriIntersection,         // visualize number of performed ray-triangle intersections
    RayTriIntersectionPassed,   // visualize number of passed ray-triangle intersections
#endif // RT_ENABLE_INTERSECTION_COUNTERS
};

enum class TraversalMode : Uint8
{
    Single = 0,
    Simd,
    Packet,
};

struct RenderingParams
{
    // Antialiasing factor: 1.0 is most optimal
    // Setting to values above 1 will blur the image
    Float antiAliasingSpread;

    // number of primary rays to be generated for image pixel
    Uint32 samplesPerPixel;

    // maximum ray depth
    Uint32 maxRayDepth;

    // ray depth after which Russian Roulette algorithm kicks in
    Uint32 minRussianRouletteDepth;

    // rendering tile dimensions (tiles are processed as a tasks in thread pool in parallel)
    Uint8 tileOrder;

    // select mode of ray traversal
    TraversalMode traversalMode;

    // allows to enable debug rendering mode
    RenderingMode renderingMode;

    RenderingParams()
        : maxRayDepth(5)
        , tileOrder(4)
        , minRussianRouletteDepth(40)
        , samplesPerPixel(1)
        , antiAliasingSpread(0.50f)
        , traversalMode(TraversalMode::Single)
        , renderingMode(RenderingMode::Regular)
    { }
};


/**
 * A structure with local (per-thread) data.
 * It's like a hub for all global params (read only) and local state (read write).
 */
struct RT_ALIGN(64) RenderingContext
{
    // two packets because of buffering
    RayPacket rayPackets[2];

    HitPoint_Packet hitPoints;

    // TODO separate stacks for scene and mesh
    Uint8 activeRaysMask[RayPacket::MaxNumGroups];
    Uint16 activeGroupsIndices[RayPacket::MaxNumGroups];

    // per-thread pseudo-random number generator
    math::Random randomGenerator;

    // global rendering parameters
    const RenderingParams* params;

    // per-thread counters
    RayTracingCounters counters;

    // counters used in local ray traversal routines
    LocalCounters localCounters;

    // for motion blur sampling
    float time;

    RT_FORCE_INLINE RenderingContext(const RenderingParams* params = nullptr)
        : params(params)
    { }
};


} // namespace rt
