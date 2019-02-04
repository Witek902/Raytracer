#pragma once

#include "Light.h"

namespace rt {

class ITexture;
using TexturePtr = std::shared_ptr<ITexture>;

class AreaLight : public ILight
{
public:
    RAYLIB_API AreaLight(const math::Vector4& p0, const math::Vector4& edge0, const math::Vector4& edge1, const math::Vector4& color);

    virtual const math::Box GetBoundingBox() const override;
    virtual bool TestRayHit(const math::Ray& ray, Float& outDistance) const override;
    virtual const RayColor Illuminate(IlluminateParam& param) const override;
    const math::Vector4 GetNormal(const math::Vector4& hitPoint) const override;
    virtual const RayColor GetRadiance(RenderingContext& context, const math::Vector4& rayDirection, const math::Vector4& hitPoint, Float* outDirectPdfA, Float* outEmissionPdfW) const override;
    virtual const RayColor Emit(RenderingContext& context, EmitResult& outResult) const override;
    virtual bool IsFinite() const override final;
    virtual bool IsDelta() const override final;

    TexturePtr mTexture = nullptr;

private:
    math::Vector4 p0;
    math::Vector4 edge0;
    math::Vector4 edge1;
    math::Vector4 normal;

    Float invArea; // inverted surface area
    Float edgeLengthInv0;
    Float edgeLengthInv1;

    bool isTriangle = false;
};

} // namespace rt
