#pragma once

#include "../RayLib.h"
#include "../Math/Vector4.h"

namespace rt {

// Represents spectral power distribution (SPD)
struct Spectrum
{
    math::Vector4 rgbValues;

    RT_FORCE_INLINE Spectrum() = default;
    RT_FORCE_INLINE Spectrum(const math::Vector4& rgbValues) : rgbValues(rgbValues) { }
};


} // namespace rt
