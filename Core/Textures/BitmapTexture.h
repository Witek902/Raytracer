#pragma once

#include "Texture.h"

namespace rt {

namespace math {
class Distribution;
}

class Bitmap;
using BitmapPtr = std::shared_ptr<Bitmap>;

enum class BitmapTextureFilter : uint8
{
    NearestNeighbor = 0,
    Bilinear = 1,
    Bilinear_SmoothStep = 2,
};

// texture wrapper for Bitmap class
class BitmapTexture : public ITexture
{
public:
    RAYLIB_API BitmapTexture();
    RAYLIB_API BitmapTexture(const BitmapPtr& bitmap);
    ~BitmapTexture();

    virtual const char* GetName() const override;
    virtual const math::Vector4 Evaluate(const math::Vector4& coords) const override;
    virtual const math::Vector4 Sample(const math::Float2 u, math::Vector4& outCoords, float* outPdf) const override;

    virtual bool MakeSamplable() override;
    virtual bool IsSamplable() const override;

private:
    BitmapPtr mBitmap;
    std::unique_ptr<math::Distribution> mImportanceMap;
    BitmapTextureFilter mFilter;
    bool mForceLinearSpace;
};

} // namespace rt
