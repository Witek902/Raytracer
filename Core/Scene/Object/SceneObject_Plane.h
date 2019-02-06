#pragma once

#include "SceneObject.h"

namespace rt {

class Material;

// Primite scene object - infinite plane
// Note: Normal is Y+
class PlaneSceneObject : public ISceneObject
{
public:
    RAYLIB_API PlaneSceneObject(const math::Float2 size = math::Float2(FLT_MAX), const math::Float2 texScale = math::Float2(1.0f));

private:
    virtual math::Box GetBoundingBox() const override;

    bool Traverse_Single_Internal(const SingleTraversalContext& context, float& outDist) const;

    virtual void Traverse_Single(const SingleTraversalContext& context, const Uint32 objectID) const override;
    virtual void Traverse_Packet(const PacketTraversalContext& context, const Uint32 objectID, const Uint32 numActiveGroups) const override;

    virtual bool Traverse_Shadow_Single(const SingleTraversalContext& context) const override;

    virtual void EvaluateShadingData_Single(const HitPoint& intersechitPointtionData, ShadingData& outShadingData) const override;

    math::Float2 mSize;
    math::Float2 mTextureScale;
};

} // namespace rt
