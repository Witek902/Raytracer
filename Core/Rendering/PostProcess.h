#pragma once

#include "../RayLib.h"

#include "../Math/Vector4.h"
#include "../Color/ColorHelpers.h"

namespace rt {

struct RT_ALIGN(16) PostprocessParams
{
    math::Vector4 colorFilter;

    float exposure;             // exposure in log scale
    float contrast;
    float saturation;
    float ditheringStrength;    // applied after tonemapping
    float bloomFactor;          // bloom multiplier

    // tonemapping curve
    Tonemapper tonemapper = Tonemapper::ACES;

    RAYLIB_API PostprocessParams();


    RAYLIB_API bool operator == (const PostprocessParams& other) const;
    RAYLIB_API bool operator != (const PostprocessParams& other) const;
};

} // namespace rt
