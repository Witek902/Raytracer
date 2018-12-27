#pragma once

#include "SceneObject.h"

namespace rt {

class Material;

// Primite scene object - sphere
class RAYLIB_API SphereSceneObject : public ISceneObject
{
public:
    SphereSceneObject(const float radius);

private:
    virtual math::Box GetBoundingBox() const override;

    virtual void Traverse_Single(const SingleTraversalContext& context, const Uint32 objectID) const override;
    virtual void Traverse_Packet(const PacketTraversalContext& context, const Uint32 objectID, const Uint32 numActiveGroups) const override;

    virtual bool Traverse_Shadow_Single(const SingleTraversalContext& context) const override;

    virtual void EvaluateShadingData_Single(const HitPoint& intersechitPointtionData, ShadingData& outShadingData) const override;

    float mRadius;
    float mInvRadius;
};

} // namespace rt
