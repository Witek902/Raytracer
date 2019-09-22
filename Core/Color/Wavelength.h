#pragma once

#include "../RayLib.h"
#include "../Config.h"

#ifdef RT_ENABLE_SPECTRAL_RENDERING
#include "../Math/Vector8.h"
#else
#include "../Math/Vector4.h"
#endif

namespace rt {

// Represents ray wavelength(s), randomized for primary rays
struct Wavelength
{
    static constexpr float Lower = 0.380e-6f;
    static constexpr float Higher = 0.720e-6f;

#ifdef RT_ENABLE_SPECTRAL_RENDERING
    static constexpr uint32 NumComponents = 8;
    using ValueType = math::Vector8;
#else
    static constexpr uint32 NumComponents = 4;
    using ValueType = math::Vector4;
#endif

    ValueType value;

#ifdef RT_ENABLE_SPECTRAL_RENDERING
    bool isSingle = false;
#endif

    RAYLIB_API void Randomize(float u);

    RT_FORCE_INLINE float GetBase() const
    {
        return value[0];
    }
};


} // namespace rt
