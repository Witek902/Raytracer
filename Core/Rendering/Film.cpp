#include "PCH.h"
#include "Film.h"
#include "../Utils/Bitmap.h"
#include "../Math/Random.h"

namespace rt {

using namespace math;

Film::Film(Bitmap& sum, Bitmap* secondarySum)
    : mFilmSize((float)sum.GetWidth(), (float)sum.GetHeight())
    , mSum(sum)
    , mSecondarySum(secondarySum) 
    , mWidth(mSum.GetWidth())
    , mHeight(mSum.GetHeight())
{
    if (mSecondarySum)
    {
        RT_ASSERT(mSecondarySum->GetWidth() == mWidth);
        RT_ASSERT(mSecondarySum->GetHeight() == mHeight);
    }
}

void Film::AccumulateColor(const Uint32 x, const Uint32 y, const Vector4& sampleColor)
{
    RT_ASSERT(x < mSum.GetWidth());
    RT_ASSERT(y < mSum.GetHeight());

    const size_t pixelIndex = mWidth * y + x;

    mSum.GetDataAs<Float3>()[pixelIndex] += sampleColor.ToFloat3();

    if (mSecondarySum)
    {
        mSecondarySum->GetDataAs<Float3>()[pixelIndex] += sampleColor.ToFloat3();
    }
}

void Film::AccumulateColor(const Vector4& pos, const Vector4& sampleColor, Random& randomGenerator)
{
    const Vector4 filmCoords = pos * mFilmSize + Vector4(0.0f, 0.5f);
    VectorInt4 intFilmCoords = VectorInt4::Convert(filmCoords);

    // apply jitter to simulate box filter
    // Note: could just splat to 4 nearest pixels, but may be slower
    {
        const Vector4 coordFraction = filmCoords - intFilmCoords.ConvertToFloat();
        const Vector4 u = randomGenerator.GetVector4();

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
        const size_t pixelIndex = mWidth * y + x;

        mSum.GetDataAs<Float3>()[pixelIndex] += sampleColor.ToFloat3();

        if (mSecondarySum)
        {
            mSecondarySum->GetDataAs<Float3>()[pixelIndex] += sampleColor.ToFloat3();
        }
    }
}

} // namespace rt
