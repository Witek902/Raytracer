#pragma once

#include "SceneObject.h"

namespace rt {

class ILight;
using LightPtr = std::unique_ptr<ILight>;

class LightSceneObject : public ISceneObject
{
public:
    RAYLIB_API explicit LightSceneObject(LightPtr light);

    virtual Type GetType() const override;

    RT_FORCE_INLINE const ILight& GetLight() const { return *mLight; }

private:
    virtual math::Box GetBoundingBox() const override;

    virtual void Traverse(const SingleTraversalContext& context, const Uint32 objectID) const override;
    virtual void Traverse(const PacketTraversalContext& context, const Uint32 objectID, const Uint32 numActiveGroups) const override;

    virtual bool Traverse_Shadow(const SingleTraversalContext& context) const override;

    virtual void EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outIntersectionData) const override;

    LightPtr mLight;
};

} // namespace rt
