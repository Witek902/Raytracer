#pragma once

#include "Light.h"

namespace rt {

class ITexture;
using TexturePtr = std::shared_ptr<ITexture>;

class BackgroundLight : public ILight
{
public:
    RAYLIB_API BackgroundLight() = default;

    RAYLIB_API BackgroundLight(const math::Vector4& color)
        : ILight(color)
    {}

    TexturePtr mTexture = nullptr;

    virtual const math::Box GetBoundingBox() const override;
    virtual bool TestRayHit(const math::Ray& ray, float& outDistance) const override;
    virtual const RayColor Illuminate(const IlluminateParam& param, IlluminateResult& outResult) const override;
    virtual const RayColor GetRadiance(RenderingContext& context, const math::Ray& ray, const math::Vector4& hitPoint, float* outDirectPdfA, float* outEmissionPdfW) const override;
    virtual const RayColor Emit(const EmitParam& param, EmitResult& outResult) const override;
    virtual bool IsFinite() const override final;
    virtual bool IsDelta() const override final;

    const RayColor GetBackgroundColor(const math::Vector4& dir, const Wavelength& wavelength) const;
};

} // namespace rt
