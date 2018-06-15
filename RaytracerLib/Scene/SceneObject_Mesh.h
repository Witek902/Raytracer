#pragma once

#include "SceneObject.h"

#include <memory>

namespace rt {

namespace math {
class Ray;
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
    virtual void Traverse_Single(const Uint32 objectID, const math::Ray& ray, HitPoint& hitPoint) const override;
    virtual void Traverse_Simd8(const math::Ray_Simd8& ray, HitPoint_Simd8& outHitPoint) const override;
    virtual void Traverse_Packet(const RayPacket& rayPacket, HitPoint_Packet& outHitPoint) const override;
    virtual void EvaluateShadingData_Single(const math::Matrix& worldToLocal, const HitPoint& hitPoint, ShadingData& outShadingData) const override;
};

} // namespace rt
