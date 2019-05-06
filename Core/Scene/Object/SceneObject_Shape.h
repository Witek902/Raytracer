#pragma once

#include "SceneObject.h"

namespace rt {

class IShape;
using ShapePtr = std::shared_ptr<IShape>;

class ShapeSceneObject : public ISceneObject
{
public:
    RAYLIB_API ShapeSceneObject(const ShapePtr& shape);

    virtual Type GetType() const override;

    RAYLIB_API void SetDefaultMaterial(const MaterialPtr& material);

private:
    virtual math::Box GetBoundingBox() const override;

    virtual void Traverse(const SingleTraversalContext& context, const Uint32 objectID) const override;
    virtual void Traverse(const PacketTraversalContext& context, const Uint32 objectID, const Uint32 numActiveGroups) const override;

    virtual bool Traverse_Shadow(const SingleTraversalContext& context) const override;

    virtual void EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outIntersectionData) const override;

    ShapePtr mShape;

    MaterialPtr mDefaultMaterial;
};

using ShapeSceneObjectPtr = std::unique_ptr<ShapeSceneObject>;

} // namespace rt
