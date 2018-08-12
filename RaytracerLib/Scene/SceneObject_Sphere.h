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

    virtual void Traverse_Single(const SingleTraversalContext& context, const Uint32 objectID) const override;
    virtual void Traverse_Simd8(const SimdTraversalContext& context, const Uint32 objectID) const override;
    virtual void Traverse_Packet(const PacketTraversalContext& context, const Uint32 objectID) const override;

    virtual void EvaluateShadingData_Single(const math::Matrix& worldToLocal, const HitPoint& intersechitPointtionData, ShadingData& outShadingData) const override;
};

} // namespace rt
