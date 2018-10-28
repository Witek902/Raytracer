#pragma once

#include "Light.h"

namespace rt {

class RAYLIB_API DirectionalLight : public ILight
{
public:
    DirectionalLight() = default;

    DirectionalLight(const math::Vector4& direction, const math::Vector4& color)
        : direction(direction.Normalized3())
        , color(color)
    {}

    math::Vector4 direction; // incident light direction
    math::Vector4 color;

    virtual const math::Box GetBoundingBox() const override;
    virtual bool TestRayHit(const math::Ray& ray, Float& outDistance) const override;
    virtual const Color Illuminate(const math::Vector4& scenePoint, RenderingContext& context, math::Vector4& outDirectionToLight, float& outDistance, float& outDirectPdfW) const override;
    virtual const Color GetRadiance(RenderingContext& context, const math::Vector4& rayDirection, const math::Vector4& hitPoint, Float* outDirectPdfA) const override;
    virtual bool IsFinite() const override final;
    virtual bool IsDelta() const override final;
};

} // namespace rt
