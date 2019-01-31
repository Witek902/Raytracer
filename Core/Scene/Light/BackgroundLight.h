#pragma once

#include "Light.h"

namespace rt {

class Bitmap;
using BitmapPtr = std::shared_ptr<Bitmap>;

class RAYLIB_API BackgroundLight : public ILight
{
public:
    BackgroundLight() = default;

    BackgroundLight(const math::Vector4& color)
        : ILight(color)
    {}

    BitmapPtr mTexture = nullptr;

    virtual const math::Box GetBoundingBox() const override;
    virtual bool TestRayHit(const math::Ray& ray, Float& outDistance) const override;
    virtual const RayColor Illuminate(IlluminateParam& param) const override;
    virtual const RayColor GetRadiance(RenderingContext& context, const math::Vector4& rayDirection, const math::Vector4& hitPoint, Float* outDirectPdfA) const override;
    virtual bool IsFinite() const override final;
    virtual bool IsDelta() const override final;

    const RayColor GetBackgroundColor(const math::Vector4& dir, RenderingContext& context) const;
};

} // namespace rt
