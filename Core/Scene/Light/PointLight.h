#pragma once

#include "Light.h"

namespace rt {

class PointLight : public ILight
{
public:
    RAYLIB_API PointLight(const math::Vector4& color);

    virtual Type GetType() const override;
    virtual const math::Box GetBoundingBox() const override;
    virtual bool TestRayHit(const math::Ray& ray, float& outDistance) const override;
    virtual const RayColor Illuminate(const IlluminateParam& param, IlluminateResult& outResult) const override;
    virtual const RayColor Emit(const EmitParam& param, EmitResult& outResult) const override;
    virtual Flags GetFlags() const override final;

private:

    // TODO texture
};

} // namespace rt
