#pragma once

#include "../RayLib.h"
#include "../Math/Box.h"
#include "../Math/Transform.h"
#include "../Utils/AlignmentAllocator.h"
#include "../Traversal/HitPoint.h"

#include <memory>

namespace rt {

struct HitPoint;
struct ShadingData;
struct SingleTraversalContext;
struct SimdTraversalContext;
struct PacketTraversalContext;

// Object on a scene
class RAYLIB_API ISceneObject : public Aligned<16>
{
public:
    virtual ~ISceneObject();

    // traverse the object and return hit points
    virtual void Traverse_Single(const SingleTraversalContext& context, const Uint32 objectID) const = 0;
    virtual void Traverse_Simd8(const SimdTraversalContext& context, const Uint32 objectID) const = 0;
    virtual void Traverse_Packet(const PacketTraversalContext& context, const Uint32 objectID) const = 0;

    // Calculate input data for shading routine
    // NOTE: all calculations are performed in local space
    virtual void EvaluateShadingData_Single(const HitPoint& hitPoint, ShadingData& outShadingData) const = 0;

    // Get world-space bounding box
    virtual math::Box GetBoundingBox() const = 0;

    math::Transform GetTransform(const float t) const;
    math::Transform GetInverseTransform(const float t) const;

    math::Transform mTransform;         // local->world transform
    math::Transform mTransformDelta;    // local transform delta for motion blur
};

using SceneObjectPtr = std::unique_ptr<ISceneObject>;

} // namespace rt
