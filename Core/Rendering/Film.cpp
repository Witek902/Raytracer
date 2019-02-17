#include "PCH.h"
#include "Film.h"
#include "../Utils/Bitmap.h"
#include "../Math/Random.h"

namespace rt {

using namespace math;

Film::Film(Bitmap& sum, Bitmap& secondarySum, const Vector4 filmSize)
    : mSum(sum)
    , mSecondarySum(secondarySum)
    , mFilmSize(filmSize)
{
    mWidth = mSum.GetWidth();
    mHeight = mSum.GetHeight();
}

void Film::AccumulateColor(const Uint32 x, const Uint32 y, const Vector4& sampleColor)
{
    const Uint32 width = mSum.GetWidth();
    Float3* __restrict sumPixels = mSum.GetDataAs<Float3>();

    const size_t pixelIndex = width * y + x;
    sumPixels[pixelIndex] += sampleColor.ToFloat3();
}

void Film::AccumulateColor(const Vector4 pos, const Vector4 sampleColor, Random& randomGenerator)
{
    const Vector4 filmCoords = pos * mFilmSize + Vector4(0.0f, 0.5f);
    VectorInt4 intFilmCoords = VectorInt4::Convert(filmCoords);

    // apply jitter to simulate box filter
    // Note: could just splat to 4 nearest pixels, but may be slower
    {
        const Vector4 coordFraction = filmCoords - intFilmCoords.ConvertToFloat();
        const Float2 u = randomGenerator.GetFloat2();

        if (u.x < coordFraction.x)
        {
            intFilmCoords.x++;
        }

        if (u.y < coordFraction.y)
        {
            intFilmCoords.y++;
        }
    }

    const Int32 x = intFilmCoords.x;
    const Int32 y = Int32(mHeight - 1) - Int32(filmCoords.y);

    if (x >= 0 && y >= 0 && x < Int32(mWidth) && y < Int32(mHeight))
    {
        Float3* __restrict sumPixels = mSum.GetDataAs<Float3>();

        const size_t pixelIndex = mWidth * y + x;
        sumPixels[pixelIndex] += sampleColor.ToFloat3();
    }
}

} // namespace rt
