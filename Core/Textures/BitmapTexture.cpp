#include "PCH.h"
#include "BitmapTexture.h"
#include "../Utils/Bitmap.h"
#include "../Utils/Logger.h"
#include "../Math/Distribution.h"
#include "../Containers/DynArray.h"
#include "../Color/ColorHelpers.h"

namespace rt {

using namespace math;

BitmapTexture::BitmapTexture() = default;
BitmapTexture::~BitmapTexture() = default;

BitmapTexture::BitmapTexture(const BitmapPtr& bitmap)
    : mBitmap(bitmap)
    , mFilter(BitmapTextureFilter::NearestNeighbor)
    , mForceLinearSpace(false)
{}

const char* BitmapTexture::GetName() const
{
    if (!mBitmap)
    {
        return "<none>";
    }

    return mBitmap->GetDebugName();
}

const Vector4 BitmapTexture::Evaluate(const Vector4& coords) const
{
    const Bitmap* bitmapPtr = mBitmap.get();

    if (!bitmapPtr)
    {
        return Vector4::Zero();
    }

    // bitmap size
    const VectorInt4 size = VectorInt4(bitmapPtr->mWidth, bitmapPtr->mHeight, 0, 0).Swizzle<0,1,0,1>();

    // wrap to 0..1 range
    const Vector4 warpedCoords = Vector4::Mod1(coords);

    // compute texel coordinates
    const Vector4 scaledCoords = warpedCoords * bitmapPtr->mFloatSize;
    const VectorInt4 intCoords = VectorInt4::Convert(Vector4::Floor(scaledCoords));

    VectorInt4 texelCoords = intCoords;
    texelCoords -= VectorInt4::AndNot(intCoords < size, size);
    texelCoords += size & (intCoords < VectorInt4::Zero());

    Vector4 result;

    if (mFilter == BitmapTextureFilter::NearestNeighbor)
    {
        result = bitmapPtr->GetPixel(texelCoords.x, texelCoords.y, mForceLinearSpace);
    }
    else if (mFilter == BitmapTextureFilter::Bilinear || mFilter == BitmapTextureFilter::Bilinear_SmoothStep)
    {
        texelCoords = texelCoords.Swizzle<0, 1, 0, 1>();
        texelCoords += VectorInt4(0, 0, 1, 1);

        // wrap secondary coordinates
        texelCoords -= VectorInt4::AndNot(texelCoords < size, size);

        Vector4 colors[4];
        bitmapPtr->GetPixelBlock(texelCoords, colors, mForceLinearSpace);

        // bilinear interpolation
        Vector4 weights = scaledCoords - intCoords.ConvertToFloat();

        if (mFilter == BitmapTextureFilter::Bilinear_SmoothStep)
        {
            weights = SmoothStep(weights);
        }

        const Vector4 value0 = Vector4::Lerp(colors[0], colors[2], weights.SplatY());
        const Vector4 value1 = Vector4::Lerp(colors[1], colors[3], weights.SplatY());
        result = Vector4::Lerp(value0, value1, weights.SplatX());
    }
    else
    {
        RT_FATAL("Invalid bitmap filter mode");
        result = Vector4::Zero();
    }

    RT_ASSERT(result.IsValid());

    return result;
}

const Vector4 BitmapTexture::Sample(const Float2 u, Vector4& outCoords, float* outPdf) const
{
    RT_ASSERT(mImportanceMap, "Bitmap texture is not samplable");

    float pdf = 0.0f;
    const Uint32 pixelIndex = mImportanceMap->SampleDiscrete(u.x, pdf);

    const Uint32 width = mBitmap->GetWidth();
    const Uint32 height = mBitmap->GetHeight();

    // TODO get rid of division
    const Uint32 x = pixelIndex % width;
    const Uint32 y = pixelIndex / width;
    RT_ASSERT(x < width);
    RT_ASSERT(y < height);

    // TODO this is redundant, because BitmapTexture::Evaluate multiplies coords by size again...
    outCoords = Vector4::FromIntegers(x, y, 0, 0) / mBitmap->mFloatSize;

    if (outPdf)
    {
        *outPdf = pdf;
    }

    return BitmapTexture::Evaluate(outCoords);
}

bool BitmapTexture::MakeSamplable()
{
    if (mImportanceMap)
    {
        return true;
    }

    if (!mBitmap)
    {
        RT_LOG_ERROR("BitmapTexture: Failed to build importance map, because bitmap is invalid");
        return false;
    }

    RT_LOG_INFO("BitmapTexture: Generating importance map for bitmap '%s'...", mBitmap->GetDebugName());

    const Uint32 width = mBitmap->GetWidth();
    const Uint32 height = mBitmap->GetHeight();

    DynArray<float> importancePdf;
    importancePdf.Resize(width * height);

    for (Uint32 j = 0; j < height; ++j)
    {
        for (Uint32 i = 0; i < width; ++i)
        {
            const Vector4 value = mBitmap->GetPixel(i, j);
            importancePdf[width * j + i] = Vector4::Dot3(c_rgbIntensityWeights, value);
        }
    }

    mImportanceMap = std::make_unique<math::Distribution>();
    return mImportanceMap->Initialize(importancePdf.Data(), importancePdf.Size());
}

bool BitmapTexture::IsSamplable() const
{
    return mImportanceMap != nullptr;
}

} // namespace rt