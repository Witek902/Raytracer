#pragma once

#include "SceneObject.h"

#include <memory>

namespace rt {

namespace math {
class Ray_Simd8;
}

class Mesh;
class Material;
struct HitPoint;
struct HitPoint_Simd8;
struct ShadingData;
struct RayPacket;

class RAYLIB_API MeshSceneObject : public ISceneObject
{
public:
    explicit MeshSceneObject(const Mesh* mesh);

    const Mesh* mMesh;

    // TODO:
    // material modifiers
    // material mapping

private:
    virtual math::Box GetBoundingBox() const override;

    virtual void Traverse_Single(const SingleTraversalContext& context, const Uint32 objectID) const override;
    virtual void Traverse_Simd8(const SimdTraversalContext& context, const Uint32 objectID) const override;
    virtual void Traverse_Packet(const PacketTraversalContext& context, const Uint32 objectID) const override;

    virtual bool Traverse_Shadow_Single(const SingleTraversalContext& context) const override;

    virtual void EvaluateShadingData_Single(const HitPoint& hitPoint, ShadingData& outShadingData) const override;
};

} // namespace rt
