#pragma once

#include "../../RayLib.h"
#include "../../Math/Box.h"
#include "../../Math/Matrix4.h"
#include "../../Utils/Memory.h"
#include "../../Traversal/HitPoint.h"

#include <memory>

namespace rt {

struct HitPoint;
struct IntersectionData;
struct SingleTraversalContext;
struct PacketTraversalContext;

class Material;
using MaterialPtr = std::shared_ptr<rt::Material>;

// Object on a scene
class ISceneObject : public Aligned<16>
{
public:
    enum class Type : Uint8
    {
        Shape,
        Light,
        Decal,
    };

    RAYLIB_API ISceneObject();
    RAYLIB_API virtual ~ISceneObject();

    virtual Type GetType() const = 0;

    RAYLIB_API void SetTransform(const math::Matrix4& matrix);

    // Get world-space bounding box
    virtual math::Box GetBoundingBox() const = 0;

    // get transform at time=0
    RT_FORCE_INLINE const math::Matrix4& GetBaseTransform() const { return mTransform; }
    RT_FORCE_INLINE const math::Matrix4& GetBaseInverseTransform() const { return mInverseTranform; }

    // get transform at given point in time
    const math::Matrix4 GetTransform(const float t) const;
    const math::Matrix4 GetInverseTransform(const float t) const;

private:
    math::Matrix4 mTransform; // local->world transform at time=0.0
    math::Matrix4 mInverseTranform;

    // TODO velocity
};

class ITraceableSceneObject : public ISceneObject
{
public:
    // traverse the object and return hit points
    virtual void Traverse(const SingleTraversalContext& context, const Uint32 objectID) const = 0;
    virtual void Traverse(const PacketTraversalContext& context, const Uint32 objectID, const Uint32 numActiveGroups) const = 0;

    // check shadow ray occlusion
    virtual bool Traverse_Shadow(const SingleTraversalContext& context) const = 0;

    // Calculate input data for shading routine
    // NOTE: all calculations are performed in local space
    // NOTE: frame[3] (translation) will be already filled, because it can be always calculated from ray distance
    virtual void EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outIntersectionData) const = 0;
};

using SceneObjectPtr = std::unique_ptr<ISceneObject>;

} // namespace rt
