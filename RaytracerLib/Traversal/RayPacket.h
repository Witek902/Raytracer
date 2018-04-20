#pragma once

#include "../Math/Ray.h"
#include "../Math/Simd4Ray.h"
#include "../Math/Simd8Ray.h"


namespace rt {

// maximum number of rays in ray stream
// TODO experiment with this value
static const Uint32 MaxRayPacketSize = 1024;

struct ImageLocationInfo
{
    Uint16 x;
    Uint16 y;
};

// packet of coherent rays
struct RT_ALIGN(16) RayPacket
{
    // rays to cast
    math::Ray rays[MaxRayPacketSize];

    // rays influence on the image (e.g. 1.0 for primary rays)
    // TODO: 3 half-floats should be fine
    math::Vector4 weights[MaxRayPacketSize];

    // corresponding image pixels
    ImageLocationInfo imageLocations[MaxRayPacketSize];

    Uint32 numRays;

    RayPacket()
        : numRays(0)
    { }

    void PushRay(const math::Ray& ray, const math::Vector4& weight, const ImageLocationInfo& imageLocation);
};

// packet of coherent rays (8-SIMD version)
struct RT_ALIGN(32) RayPacket_Simd8
{
    static constexpr Uint32 MaxNumGroups = MaxRayPacketSize / 8;

    // rays to cast
    math::Ray_Simd8 rays[MaxNumGroups];

    // rays influence on the image (e.g. 1.0 for primary rays)
    // TODO: 3 half-floats should be fine
    math::Vector3_Simd8 weights[MaxNumGroups];

    // corresponding image pixels
    ImageLocationInfo imageLocations[MaxRayPacketSize];

    // number of rays (not groups!)
    Uint32 numRays;
};


} // namespace rt
