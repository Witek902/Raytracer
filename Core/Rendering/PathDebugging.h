#pragma once

#include "ShadingData.h"
#include "../Material/BSDF/BSDF.h"

namespace rt {


enum class PathTerminationReason
{
    None = 0,
    HitBackground,
    HitLight,
    Depth,
    Throughput,
    NoSampledEvent,
    RussianRoulette,
};

enum class RaySource : uint8
{
    Eye = 0,
    Reflection,
    Refraction,
};

struct PathDebugData
{
    struct RT_ALIGN(32) HitPointData
    {
        // ray traced
        math::Vector4 rayOrigin;
        math::Vector4 rayDir;

        HitPoint hitPoint;

        // evaluated world-space shading data
        ShadingData shadingData;

        // ray influence on the final pixel
        RayColor throughput;

        // evaluated ray color (excluding weight)
        math::Vector4 color;

        RaySource raySource;

        BSDF::EventType bsdfEvent;
    };

    PathTerminationReason terminationReason = PathTerminationReason::None;
    DynArray<HitPointData> data;
};


} // namespace rt
