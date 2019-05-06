#pragma once

#include "Light.h"

namespace rt {

class DirectionalLight : public ILight
{
public:
    RAYLIB_API DirectionalLight() = default;

    RAYLIB_API DirectionalLight(const math::Vector4& color, const float angle = 0.2f);

    virtual Type GetType() const override;
    virtual const math::Box GetBoundingBox() const override;
    virtual bool TestRayHit(const math::Ray& ray, float& outDistance) const override;
    virtual const RayColor Illuminate(const IlluminateParam& param, IlluminateResult& outResult) const override;
    virtual const RayColor GetRadiance(const RadianceParam& param, float* outDirectPdfA, float* outEmissionPdfW) const override;
    virtual const RayColor Emit(const EmitParam& param, EmitResult& outResult) const override;
    virtual Flags GetFlags() const override final;

    const math::Vector4 SampleDirection(const math::Float2 sample, float& outPdf) const;

private:
    float mCosAngle;
    bool mIsDelta;
};

} // namespace rt
