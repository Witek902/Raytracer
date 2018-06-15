#pragma once

#include "SceneObject.h"

namespace rt {

class RAYLIB_API SphereSceneObject : public ISceneObject
{
public:
    SphereSceneObject(const float radius, const Material* material);

    const Material* mMaterial;
    float mRadius;

private:
    virtual math::Box GetBoundingBox() const override;
    virtual void Traverse_Single(const Uint32 objectID, const math::Ray& ray, HitPoint& hitPoint) const override;
    virtual void Traverse_Simd8(const math::Ray_Simd8& ray, HitPoint_Simd8& outHitPoint) const override;
    virtual void Traverse_Packet(const RayPacket& rayPacket, HitPoint_Packet& outHitPoint) const override;
    virtual void EvaluateShadingData_Single(const math::Matrix& worldToLocal, const HitPoint& intersechitPointtionData, ShadingData& outShadingData) const override;
};

} // namespace rt
