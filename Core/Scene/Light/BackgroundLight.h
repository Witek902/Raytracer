#pragma once

#include "Light.h"

namespace rt {

class ITexture;
using TexturePtr = std::shared_ptr<ITexture>;

class BackgroundLight : public ILight
{
public:
    RAYLIB_API BackgroundLight();
    RAYLIB_API BackgroundLight(const math::Vector4& color);

    TexturePtr mTexture = nullptr;

    virtual Type GetType() const override;
    virtual const math::Box GetBoundingBox() const override;
    virtual bool TestRayHit(const math::Ray& ray, float& outDistance) const override;
    virtual const RayColor Illuminate(const IlluminateParam& param, IlluminateResult& outResult) const override;
    virtual const RayColor GetRadiance(const RadianceParam& param, float* outDirectPdfA, float* outEmissionPdfW) const override;
    virtual const RayColor Emit(const EmitParam& param, EmitResult& outResult) const override;
    virtual Flags GetFlags() const override final;

    const RayColor GetBackgroundColor(const math::Vector4& dir, const Wavelength& wavelength) const;
};

} // namespace rt
