#pragma once

#include "../RayLib.h"

#include "../Color/Color.h"
#include "../Traversal/HitPoint.h"
#include "../BVH/BVH.h"

#include <vector>


namespace rt {

class ISceneObject;
class ILight;
class Bitmap;
class Camera;
struct RenderingContext;
struct HitPoint;
struct ShadingData;
struct SingleTraversalContext;
struct SimdTraversalContext;
struct PacketTraversalContext;

using SceneObjectPtr = std::unique_ptr<ISceneObject>;
using LightPtr = std::unique_ptr<ILight>;

namespace math {
class Ray;
class Ray_Simd4;
class Ray_Simd8;
} // namespace math


/**
 * Global environment settings.
 */
struct SceneEnvironment
{
    math::Vector4 backgroundColor = math::Vector4(1.0f, 1.0f, 1.0f, 0.0f);

    // optional spherical texture
    Bitmap* texture = nullptr;
};

/**
 * Rendering scene.
 * Allows for placing objects (meshes, lights, etc.) and raytracing them.
 */
class RAYLIB_API RT_ALIGN(16) Scene : public Aligned<16>
{
public:
    Scene();
    ~Scene();
    Scene(Scene&&);
    Scene& operator = (Scene&&);

    void SetEnvironment(const SceneEnvironment& env);

    void AddLight(LightPtr object);
    void AddObject(SceneObjectPtr object);

    bool BuildBVH();

    RT_FORCE_INLINE const BVH& GetBVH() const { return mBVH; }
    RT_FORCE_INLINE const std::vector<SceneObjectPtr>& GetObjects() const { return mObjects; }
    RT_FORCE_INLINE const std::vector<LightPtr>& GetLights() const { return mLights; }

    // traverse the scene, returns hit points
    void Traverse_Single(const SingleTraversalContext& context) const;
    void Traverse_Packet(const PacketTraversalContext& context) const;

    // cast shadow ray
    bool Traverse_Shadow_Single(const SingleTraversalContext& context) const;

    void ExtractShadingData(const math::Vector4& rayOrigin, const math::Vector4& rayDir, const HitPoint& hitPoint, const float time, ShadingData& outShadingData) const;

    void TraceRay_Simd8(const math::Ray_Simd8& ray, RenderingContext& context, Color* outColors) const;

    void Traverse_Leaf_Single(const SingleTraversalContext& context, const Uint32 objectID, const BVH::Node& node) const;
    void Traverse_Leaf_Simd8(const SimdTraversalContext& context, const Uint32 objectID, const BVH::Node& node) const;
    void Traverse_Leaf_Packet(const PacketTraversalContext& context, const Uint32 objectID, const BVH::Node& node, Uint32 numActiveGroups) const;

    bool Traverse_Leaf_Shadow_Single(const SingleTraversalContext& context, const BVH::Node& node) const;

    void Shade_Simd8(const math::Ray_Simd8& ray, const HitPoint_Simd8& hitPoints, RenderingContext& context, Color* outColors) const;

    // perform ray packet shading:
    // 1. apply calculated color to render target
    // 2. generate secondary rays
    RT_FORCE_NOINLINE void Shade_Packet(const RayPacket& packet, const HitPoint_Packet& hitPoints, RenderingContext& context, Bitmap& renderTarget) const;

    // sample background color
    Color GetBackgroundColor(const math::Ray& ray, RenderingContext& context) const;

private:
    Scene(const Scene&) = delete;
    Scene& operator = (const Scene&) = delete;

    void Traverse_Object_Single(const SingleTraversalContext& context, const Uint32 objectID) const;
    bool Traverse_Object_Shadow_Single(const SingleTraversalContext& context, const Uint32 objectID) const;

    SceneEnvironment mEnvironment;

    std::vector<LightPtr> mLights;

    std::vector<SceneObjectPtr> mObjects;

    // bounding volume hierarchy for scene object
    BVH mBVH;
};

} // namespace rt
