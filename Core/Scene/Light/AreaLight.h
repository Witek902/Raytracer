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
    virtual bool TestRayHit(const math::Ray& ray, float& outDistance) const override;
    virtual const RayColor Illuminate(const IlluminateParam& param, IlluminateResult& outResult) const override;
    const math::Vector4 GetNormal(const math::Vector4& hitPoint) const override;
    virtual const RayColor GetRadiance(RenderingContext& context, const math::Ray& ray, const math::Vector4& hitPoint, float* outDirectPdfA, float* outEmissionPdfW) const override;
    virtual const RayColor Emit(const EmitParam& param, EmitResult& outResult) const override;
    virtual bool IsFinite() const override final;
    virtual bool IsDelta() const override final;

    TexturePtr mTexture = nullptr;

private:
    math::Vector4 p0;
    math::Vector4 edge0;
    math::Vector4 edge1;
    math::Vector4 normal;

    float invArea; // inverted surface area
    float edgeLengthInv0;
    float edgeLengthInv1;

    bool isTriangle = false;
};

} // namespace rt
