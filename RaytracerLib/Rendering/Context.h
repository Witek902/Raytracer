#pragma once

#include "Counters.h"

#include "../Traversal/RayPacket.h"
#include "../Traversal/HitPoint.h"

#include "../Color/Color.h"

#include "../Math/Random.h"

namespace rt {

struct PathDebugData;

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
    // Antialiasing factor
    // Setting to higher values will blur the image
    Float antiAliasingSpread = 0.45f;

    // Motion blur multiplier
    // NOTE: must be in [0...1] range
    Float motionBlurStrength = 0.5f;

    // number of primary rays to be generated for image pixel
    Uint32 samplesPerPixel = 1;

    // maximum ray depth
    Uint32 maxRayDepth = 20;

    // ray depth at which Russian Roulette algorithm kicks in
    Uint32 minRussianRouletteDepth = 2;

    // rendering tile dimensions (tiles are processed as a tasks in thread pool in parallel)
    Uint8 tileOrder = 4;

    // select mode of ray traversal
    TraversalMode traversalMode = TraversalMode::Single;

    // allows to enable debug rendering mode
    RenderingMode renderingMode = RenderingMode::Regular;
};

/**
 * A structure with local (per-thread) data.
 * It's like a hub for all global params (read only) and local state (read write).
 */
struct RT_ALIGN(64) RenderingContext
{
    Wavelength wavelength;

    // two packets because of buffering
    RayPacket rayPackets[2];

    HitPoint_Packet hitPoints;

    // TODO separate stacks for scene and mesh
    Uint8 activeRaysMask[RayPacket::MaxNumGroups];
    Uint16 activeGroupsIndices[RayPacket::MaxNumGroups];

    // per-thread pseudo-random number generator
    math::Random randomGenerator;

    // global rendering parameters
    const RenderingParams* params = nullptr;

    // per-thread counters
    RayTracingCounters counters;

    // counters used in local ray traversal routines
    LocalCounters localCounters;

    // for motion blur sampling
    float time = 0.0f;

    // optional path debugging data
    PathDebugData* pathDebugData = nullptr;
};


} // namespace rt
