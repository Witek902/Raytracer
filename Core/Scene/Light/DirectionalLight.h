#pragma once

#include "Light.h"

namespace rt {

class DirectionalLight : public ILight
{
public:
    RAYLIB_API DirectionalLight() = default;

    RAYLIB_API DirectionalLight(const math::Vector4& direction, const math::Vector4& color, const Float angle = 0.2f);

    virtual const math::Box GetBoundingBox() const override;
    virtual bool TestRayHit(const math::Ray& ray, Float& outDistance) const override;
    virtual const RayColor Illuminate(IlluminateParam& param) const override;
    virtual const RayColor GetRadiance(RenderingContext& context, const math::Vector4& rayDirection, const math::Vector4& hitPoint, Float* outDirectPdfA) const override;
    virtual bool IsFinite() const override final;
    virtual bool IsDelta() const override final;

private:
    math::Vector4 mDirection; // incident light direction
    Float mCosAngle;
};

} // namespace rt
