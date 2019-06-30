#include "PCH.h"
#include "Film.h"
#include "../Utils/Bitmap.h"
#include "../Math/Random.h"
#include "../Math/Vector4Load.h"

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

RT_FORCE_INLINE static void AccumulateToFloat3(Float3& target, const Vector4& value)
{
    const Vector4 original = Vector4_Load_Float3_Unsafe(target);
    target = (original + value).ToFloat3();
}

void Film::AccumulateColor(const Uint32 x, const Uint32 y, const Vector4& sampleColor)
{
    AccumulateToFloat3(mSum.GetPixelRef<Float3>(x, y), sampleColor);

    if (mSecondarySum)
    {
        AccumulateToFloat3(mSecondarySum->GetPixelRef<Float3>(x, y), sampleColor);
    }
}

RT_FORCE_NOINLINE
void Film::AccumulateColor(const Vector4& pos, const Vector4& sampleColor, Random& randomGenerator)
{
    const Vector4 filmCoords = pos * mFilmSize + Vector4(0.0f, 0.5f);
    VectorInt4 intFilmCoords = VectorInt4::Convert(filmCoords);

    // apply jitter to simulate box filter
    // Note: could just splat to 4 nearest pixels, but may be slower
    {
        const Vector4 coordFraction = filmCoords - intFilmCoords.ConvertToFloat();
        const Vector4 u = randomGenerator.GetVector4();

        intFilmCoords = VectorInt4::Select(intFilmCoords, intFilmCoords + 1, u < coordFraction);

        //if (u.x < coordFraction.x)
        //{
        //    intFilmCoords.x++;
        //}
        //if (u.y < coordFraction.y)
        //{
        //    intFilmCoords.y++;
        //}
    }

    const Int32 x = intFilmCoords.x;
    const Int32 y = Int32(mHeight - 1) - Int32(filmCoords.y);

    if (Uint32(x) < mWidth && Uint32(y) < mHeight)
    {
        AccumulateToFloat3(mSum.GetPixelRef<Float3>(x, y), sampleColor);

        if (mSecondarySum)
        {
            AccumulateToFloat3(mSecondarySum->GetPixelRef<Float3>(x, y), sampleColor);
        }
    }
}

} // namespace rt
