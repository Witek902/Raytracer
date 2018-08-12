#pragma once

#include "../RayLib.h"

#include "SceneObject.h"

#include "../Color/Color.h"
#include "../Traversal/HitPoint.h"
#include "../BVH/BVH.h"

#include <vector>


namespace rt {

class Bitmap;
class Camera;
struct RenderingContext;
struct ShadingData;
struct LocalCounters;

namespace math {
class Ray;
class Ray_Simd4;
class Ray_Simd8;
} // namespace math

using LightID = Uint32;
using MeshInstanceID = Uint32;


/**
 * Global environment settings.
 */
struct SceneEnvironment
{
    math::Vector4 backgroundColor;

    // optional spherical texture
    Bitmap* texture;

    SceneEnvironment()
        : backgroundColor(math::Vector4(1.0f, 1.0f, 1.0f, 0.0f))
        , texture(nullptr)
    { }
};

/**
 * Rendering scene.
 * Allows for placing objects (meshes, lights, etc.) and raytracing them.
 */
class RAYLIB_API RT_ALIGN(16) Scene : public Aligned<16>
{
public:
    Scene();

    void SetEnvironment(const SceneEnvironment& env);

    void AddObject(SceneObjectPtr object);

    bool BuildBVH();

    RT_FORCE_NOINLINE const BVH& GetBVH() const { return mBVH; }

    // traverse the scene, returns hit points
    RT_FORCE_NOINLINE void Traverse_Single(const SingleTraversalContext& context) const;
    RT_FORCE_NOINLINE void Traverse_Packet(const PacketTraversalContext& context) const;

    void ExtractShadingData(const math::Vector4& rayOrigin, const math::Vector4& rayDir, const HitPoint& hitPoint, ShadingData& outShadingData) const;

    RT_FORCE_NOINLINE RayColor TraceRay_Single(const math::Ray& ray, RenderingContext& context) const;
    RT_FORCE_NOINLINE void TraceRay_Simd8(const math::Ray_Simd8& ray, RenderingContext& context, RayColor* outColors) const;

    RT_FORCE_NOINLINE void Traverse_Leaf_Single(const SingleTraversalContext& context, const Uint32 objectID, const BVH::Node& node) const;
    RT_FORCE_NOINLINE void Traverse_Leaf_Simd8(const SimdTraversalContext& context, const Uint32 objectID, const BVH::Node& node) const;
    RT_FORCE_NOINLINE void Traverse_Leaf_Packet(const PacketTraversalContext& context, const Uint32 objectID, const BVH::Node& node, Uint32 numActiveGroups) const;

    RT_FORCE_NOINLINE void Shade_Simd8(const math::Ray_Simd8& ray, const HitPoint_Simd8& hitPoints, RenderingContext& context, RayColor* outColors) const;

    // perform ray packet shading:
    // 1. apply calculated color to render target
    // 2. generate secondary rays
    RT_FORCE_NOINLINE void Shade_Packet(const RayPacket& packet, const HitPoint_Packet& hitPoints, RenderingContext& context, Bitmap& renderTarget) const;

    // sample background color
    RayColor GetBackgroundColor(const math::Ray& ray) const;

private:
    Scene(const Scene&) = delete;
    Scene& operator = (const Scene&) = delete;

    static RayColor HandleSpecialRenderingMode(RenderingContext& context, const HitPoint& hitPoint, const ShadingData& shadingData);

    SceneEnvironment mEnvironment;

    std::vector<SceneObjectPtr> mObjects;

    // bounding volume hierarchy for scene object
    BVH mBVH;
};

} // namespace rt
