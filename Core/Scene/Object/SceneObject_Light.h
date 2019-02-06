#pragma once

#include "SceneObject.h"

namespace rt {

class ILight;

// scene object representing a finite scene object (e.g. area light)
class LightSceneObject : public ISceneObject
{
public:
    RAYLIB_API explicit LightSceneObject(const ILight& light);

    RT_FORCE_INLINE const ILight& GetLight() const { return mLight; }

private:
    virtual math::Box GetBoundingBox() const override;

    virtual void Traverse_Single(const SingleTraversalContext& context, const Uint32 objectID) const override;
    virtual void Traverse_Packet(const PacketTraversalContext& context, const Uint32 objectID, const Uint32 numActiveGroups) const override;

    virtual bool Traverse_Shadow_Single(const SingleTraversalContext& context) const override;

    virtual void EvaluateShadingData_Single(const HitPoint& hitPoint, ShadingData& outShadingData) const override;

    const ILight& mLight;
};

} // namespace rt
