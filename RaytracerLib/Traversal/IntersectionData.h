#pragma once

#include "RayPacket.h"

#include <float.h>


namespace rt {

// Ray-scene intersection data (non-SIMD)
struct RayIntersectionData
{
    float distance;
    float u;
    float v;
    Uint32 triangle;
    Uint32 instance;

    RayIntersectionData()
        : distance(FLT_MAX)
        , u(0.0f)
        , v(0.0f)
        , triangle(UINT32_MAX)
        , instance(UINT32_MAX)
    {}
};

using RayPacketIntersectionData = RayIntersectionData[MaxRayPacketSize];

} // namespace rt
