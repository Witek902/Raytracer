#pragma once

#include "Light.h"

namespace rt {

class PointLight : public ILight
{
public:
    RAYLIB_API PointLight(const math::Vector4& position, const math::Vector4& color);

    virtual const math::Box GetBoundingBox() const override;
    virtual bool TestRayHit(const math::Ray& ray, float& outDistance) const override;
    virtual const RayColor Illuminate(IlluminateParam& param) const override;
    virtual const RayColor Emit(RenderingContext& context, EmitResult& outResult) const override;
    virtual bool IsFinite() const override final;
    virtual bool IsDelta() const override final; 

private:
    math::Vector4 mPosition;
};

} // namespace rt
