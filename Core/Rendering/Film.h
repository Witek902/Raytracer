#pragma once

#include "../RayLib.h"
#include "../Math/Vector4.h"

namespace rt {

class Bitmap;

class Film
{
public:
    Film::Film(Bitmap& sum, Bitmap& secondarySum, const math::Vector4 filmSize)
        : mSum(sum)
        , mSecondarySum(secondarySum)
        , mFilmSize(filmSize)
    {
    }

    void AccumulateColor(const math::Vector4 pos, const math::Vector4 sampleColor);
    void AccumulateColor(const Uint32 x, const Uint32 y, const math::Vector4& sampleColor);

private:
    math::Vector4 mFilmSize;

    Bitmap& mSum;
    Bitmap& mSecondarySum;
};

} // namespace rt
