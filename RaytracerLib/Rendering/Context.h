#pragma once

#include "Counters.h"

#include "../Traversal/RayPacket.h"
#include "../Traversal/HitPoint.h"

#include "../Color/Color.h"

#include "../Math/Random.h"

namespace rt {

struct PathDebugData;

enum class TraversalMode : Uint8
{
    Single = 0,
    Simd,
    Packet,
};

struct AdaptiveRenderingSettings
{
    Bool enable = true;
    Uint32 numInitialPasses = 10;
    Uint32 minBlockSize = 4;
    Uint32 maxBlockSize = 256;
    Float subdivisionTreshold = 0.005f;
    Float convergenceTreshold = 0.0001f;
};

struct RenderingParams
{
    // Antialiasing factor
    // Setting to higher values will blur the image
    Float antiAliasingSpread = 0.5f;

    // Motion blur multiplier
    // NOTE: must be in [0...1] range
    Float motionBlurStrength = 0.5f;

    // number of primary rays to be generated for image pixel
    Uint32 samplesPerPixel = 1;

    // maximum ray depth
    Uint32 maxRayDepth = 50;

    // ray depth at which Russian Roulette algorithm kicks in
    Uint32 minRussianRouletteDepth = 2;

    // rendering tile dimensions (tiles are processed as a tasks in thread pool in parallel)
    Uint16 tileSize = 16;

    // select mode of ray traversal
    TraversalMode traversalMode = TraversalMode::Single;

    // adaptive rendering settings
    AdaptiveRenderingSettings adaptiveSettings;
};

/**
 * A structure with local (per-thread) data.
 * It's like a hub for all global params (read only) and local state (read write).
 */
struct RT_ALIGN(64) RenderingContext : public Aligned<64>
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
