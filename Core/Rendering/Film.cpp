#include "PCH.h"
#include "Film.h"
#include "../Utils/Bitmap.h"


namespace rt {

using namespace math;

void Film::AccumulateColor(const Uint32 x, const Uint32 y, const math::Vector4& sampleColor)
{
    const Uint32 width = mSum.GetWidth();
    Float3* __restrict sumPixels = mSum.GetDataAs<Float3>();

    const size_t pixelIndex = width * y + x;
    sumPixels[pixelIndex] += sampleColor.ToFloat3();
}

void Film::AccumulateColor(const Vector4 pos, const math::Vector4 sampleColor)
{
    const Int32 width = mSum.GetWidth();
    const Int32 height = mSum.GetHeight();

    const Vector4 filmCoords = pos * mFilmSize + Vector4(0.5f);
    const VectorInt4 intFilmCoords = VectorInt4::Convert(filmCoords);

    const Int32 x = intFilmCoords.x;
    const Int32 y = height - 1 - (Int32)filmCoords.y;

    if (x >= 0 && y >= 0 && x < width && y < height)
    {
        Float3* __restrict sumPixels = mSum.GetDataAs<Float3>();

        const size_t pixelIndex = width * y + x;
        sumPixels[pixelIndex] += sampleColor.ToFloat3();
    }
}

} // namespace rt
