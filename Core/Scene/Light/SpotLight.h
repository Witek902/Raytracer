#pragma once

#include "Light.h"

namespace rt {

class SpotLight : public ILight
{
public:
    RAYLIB_API SpotLight(const math::Vector4& position, const math::Vector4& direction, const math::Vector4& color, const float angle);

    virtual Type GetType() const override;
    virtual const math::Box GetBoundingBox() const override;
    virtual bool TestRayHit(const math::Ray& ray, float& outDistance) const override;
    virtual const RayColor Illuminate(const IlluminateParam& param, IlluminateResult& outResult) const override;
    virtual const RayColor Emit(const EmitParam& param, EmitResult& outResult) const override;
    virtual bool IsFinite() const override final;
    virtual bool IsDelta() const override final;

private:
    math::Vector4 mPosition;
    math::Vector4 mDirection;
    float mAngle;
    float mCosAngle;
    bool mIsDelta; // true for 'laser' light
};

} // namespace rt
