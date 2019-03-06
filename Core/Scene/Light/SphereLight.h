#pragma once

#include "Light.h"

namespace rt {

class SphereLight : public ILight
{
public:
    RAYLIB_API SphereLight(const math::Vector4& pos, float radius, const math::Vector4& color);

    virtual const math::Box GetBoundingBox() const override;
    virtual bool TestRayHit(const math::Ray& ray, float& outDistance) const override;
    virtual const RayColor Illuminate(const IlluminateParam& param, IlluminateResult& outResult) const override;
    virtual const math::Vector4 GetNormal(const math::Vector4& hitPoint) const override;
    virtual const RayColor GetRadiance(RenderingContext& context, const math::Ray& ray, const math::Vector4& hitPoint, float* outDirectPdfA, float* outEmissionPdfW) const override;
    virtual const RayColor Emit(const EmitParam& param, EmitResult& outResult) const override;
    virtual bool IsFinite() const override final;
    virtual bool IsDelta() const override final;

private:
    math::Vector4 mPosition;
    float mRadius;
    float mRadiusSqr;
};

} // namespace rt
