#pragma once

#include "Light.h"

namespace rt {

class SphereLight : public ILight
{
public:
    RAYLIB_API SphereLight(const math::Vector4& pos, Float radius, const math::Vector4& color);

    virtual const math::Box GetBoundingBox() const override;
    virtual bool TestRayHit(const math::Ray& ray, Float& outDistance) const override;
    virtual const RayColor Illuminate(IlluminateParam& param) const override;
    virtual const math::Vector4 GetNormal(const math::Vector4& hitPoint) const override;
    virtual const RayColor GetRadiance(RenderingContext& context, const math::Vector4& rayDirection, const math::Vector4& hitPoint, Float* outDirectPdfA, Float* outEmissionPdfW) const override;
    virtual const RayColor Emit(RenderingContext& context, EmitResult& outResult) const override;
    virtual bool IsFinite() const override final;
    virtual bool IsDelta() const override final;

private:
    math::Vector4 mPosition;
    Float mRadius;
    Float mRadiusSqr;
};

} // namespace rt
