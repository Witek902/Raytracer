#pragma once

#include "../RayLib.h"
#include "../Math/Matrix.h"
#include "../Math/Box.h"
#include "../Utils/AlignmentAllocator.h"
#include "../Traversal/HitPoint.h"

#include <memory>

namespace rt {

class Material;
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
    // TODO worldToLocal is a hack. All should be calculated in local space
    virtual void EvaluateShadingData_Single(const math::Matrix& worldToLocal, const HitPoint& hitPoint, ShadingData& outShadingData) const = 0;

    // Get world-space bounding box
    virtual math::Box GetBoundingBox() const = 0;

    // TODO use Transform class (translation + quaternion)
    math::Matrix GetTransform(const float t) const;
    math::Matrix GetInverseTransform(const float t) const;

    math::Vector4 mPosition;
    math::Vector4 mPositionOffset;
};

using SceneObjectPtr = std::unique_ptr<ISceneObject>;

} // namespace rt
