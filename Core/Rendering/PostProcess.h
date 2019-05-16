#pragma once

#include "../RayLib.h"

#include "../Math/Vector4.h"

namespace rt {

struct RT_ALIGN(16) PostprocessParams
{
    math::Vector4 colorFilter;

    // exposure in log scale
    float exposure;

    // applied after tonemapping
    float ditheringStrength;

    // bloom multiplier
    float bloomFactor;

    RAYLIB_API PostprocessParams();

    RAYLIB_API bool operator == (const PostprocessParams& other) const;
    RAYLIB_API bool operator != (const PostprocessParams& other) const;
};

template<typename T>
RT_FORCE_INLINE static T ToneMap(T color)
{
    const T b = T(6.2f);
    const T c = T(1.7f);
    const T d = T(0.06f);

    // Jim Hejl and Richard Burgess-Dawson formula
    const T t0 = color * T::MulAndAdd(color, b, T(0.5f));
    const T t1 = T::MulAndAdd(color, b, c);
    const T t2 = T::MulAndAdd(color, t1, d);
    return t0 * T::FastReciprocal(t2);
}

} // namespace rt
