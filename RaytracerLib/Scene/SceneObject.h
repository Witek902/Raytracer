#pragma once

#include "../RayLib.h"
#include "../Math/Matrix.h"
#include "../Math/Box.h"

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

// Object on a scene
class RAYLIB_API ISceneObject
{
public:
    virtual ~ISceneObject();

    // traverse the object and return hit points
    virtual void Traverse_Single(const math::Ray& ray, HitPoint& hitPoint) const = 0;
    virtual void Traverse_Simd8(const math::Ray_Simd8& ray, HitPoint_Simd8& outHitPoint) const = 0;

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
    virtual void Traverse_Single(const math::Ray& ray, HitPoint& hitPoint) const override;
    virtual void Traverse_Simd8(const math::Ray_Simd8& ray, HitPoint_Simd8& outHitPoint) const override;
    virtual void EvaluateShadingData_Single(const math::Matrix& worldToLocal, const HitPoint& hitPoint, ShadingData& outShadingData) const override;
};

class RAYLIB_API SphereSceneObject : public ISceneObject
{
public:
    SphereSceneObject(const float radius, const Material* material);

    float mRadius;
    const Material* mMaterial;

private:
    virtual math::Box GetBoundingBox() const override;
    virtual void Traverse_Single(const math::Ray& ray, HitPoint& hitPoint) const override;
    virtual void Traverse_Simd8(const math::Ray_Simd8& ray, HitPoint_Simd8& outHitPoint) const override;
    virtual void EvaluateShadingData_Single(const math::Matrix& worldToLocal, const HitPoint& intersechitPointtionData, ShadingData& outShadingData) const override;
};

} // namespace rt
