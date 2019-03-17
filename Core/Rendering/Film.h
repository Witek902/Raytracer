#pragma once

#include "../RayLib.h"
#include "../Math/Vector4.h"

namespace rt {

class Bitmap;

namespace math {
class Random;
} // namespace math

class Film
{
public:
    Film(Bitmap& sum, Bitmap* secondarySum = nullptr);

    RT_FORCE_INLINE Uint32 GetWidth() const
    {
        return mWidth;
    }

    RT_FORCE_INLINE Uint32 GetHeight() const
    {
        return mHeight;
    }

    void AccumulateColor(const math::Vector4 pos, const math::Vector4 sampleColor, math::Random& randomGenerator);
    void AccumulateColor(const Uint32 x, const Uint32 y, const math::Vector4& sampleColor);

private:
    math::Vector4 mFilmSize;

    Bitmap& mSum;
    Bitmap* mSecondarySum;

    const Uint32 mWidth;
    const Uint32 mHeight;
};

} // namespace rt
