#pragma once

#include "Light.h"

namespace rt {

class RAYLIB_API PointLight : public ILight
{
public:
    PointLight() = default;

    PointLight(const math::Vector4& position, const math::Vector4& color)
        : position(position)
        , color(color)
    {}

    math::Vector4 position;
    math::Vector4 color;

    virtual const math::Box GetBoundingBox() const override;
    virtual bool TestRayHit(const math::Ray& ray, Float& outDistance) const override;
    virtual const Color Illuminate(IlluminateParam& param) const override;
    virtual const Color GetRadiance(RenderingContext& context, const math::Vector4& rayDirection, const math::Vector4& hitPoint, Float* outDirectPdfA) const override;
    virtual bool IsFinite() const override final;
    virtual bool IsDelta() const override final; 
};

} // namespace rt
