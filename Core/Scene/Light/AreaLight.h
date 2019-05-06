#pragma once

#include "Light.h"

#include "../../Math/Matrix4.h"

namespace rt {

class IShape;
using ShapePtr = std::shared_ptr<IShape>;

class ITexture;
using TexturePtr = std::shared_ptr<ITexture>;

// TODO unify with ShapeSceneObject
class AreaLight : public ILight
{
public:
    RAYLIB_API AreaLight(ShapePtr shape, const math::Vector4& color);

    RT_FORCE_INLINE const ShapePtr& GetShape() const { return mShape; }

    virtual Type GetType() const override;
    virtual const math::Box GetBoundingBox() const override;
    virtual bool TestRayHit(const math::Ray& ray, float& outDistance) const override;
    virtual const RayColor Illuminate(const IlluminateParam& param, IlluminateResult& outResult) const override;
    virtual const RayColor GetRadiance(const RadianceParam& param, float* outDirectPdfA, float* outEmissionPdfW) const override;
    virtual const RayColor Emit(const EmitParam& param, EmitResult& outResult) const override;
    virtual Flags GetFlags() const override final;

    TexturePtr mTexture = nullptr;

private:
    ShapePtr mShape;
};

} // namespace rt
