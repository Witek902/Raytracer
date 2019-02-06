#pragma once

#include "../../RayLib.h"
#include "../../Math/Box.h"
#include "../../Math/Matrix4.h"
#include "../../Utils/AlignmentAllocator.h"
#include "../../Traversal/HitPoint.h"

#include <memory>

namespace rt {

struct HitPoint;
struct ShadingData;
struct SingleTraversalContext;
struct PacketTraversalContext;

class Material;
using MaterialPtr = std::shared_ptr<rt::Material>;

// Object on a scene
class ISceneObject : public Aligned<16>
{
public:
    RAYLIB_API ISceneObject();
    RAYLIB_API virtual ~ISceneObject();

    // traverse the object and return hit points
    virtual void Traverse_Single(const SingleTraversalContext& context, const Uint32 objectID) const = 0;
    virtual void Traverse_Packet(const PacketTraversalContext& context, const Uint32 objectID, const Uint32 numActiveGroups) const = 0;

    // check shadow ray occlusion
    virtual bool Traverse_Shadow_Single(const SingleTraversalContext& context) const = 0;

    // Calculate input data for shading routine
    // NOTE: all calculations are performed in local space
    virtual void EvaluateShadingData_Single(const HitPoint& hitPoint, ShadingData& outShadingData) const = 0;

    // Get world-space bounding box
    virtual math::Box GetBoundingBox() const = 0;

    const math::Matrix4 ComputeTransform(const float t) const;

    // local->world transform at time=0.0
    math::Matrix4 mTransform;

    // TODO velocity

    MaterialPtr mDefaultMaterial;
};

using SceneObjectPtr = std::unique_ptr<ISceneObject>;

} // namespace rt
