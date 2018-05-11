#pragma once

#include "../RayLib.h"

#include "SceneObject.h"

#include "../Color/RayColor.h"
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
        : backgroundColor(math::Vector4(1.0f, 1.0f, 1.0f))
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

    // traverse the scene, returns hit points
    RT_FORCE_NOINLINE void Traverse_Single(const math::Ray& ray, HitPoint& outHitPoint, RenderingContext& context) const;
    RT_FORCE_NOINLINE void Traverse_Simd8(const math::Ray_Simd8& ray, HitPoint_Simd8& outHitPoint, RenderingContext& context) const;

    void ExtractShadingData(const math::Vector4& rayOrigin, const math::Vector4& rayDir, const HitPoint& hitPoint, ShadingData& outShadingData) const;

    RT_FORCE_NOINLINE RayColor TraceRay_Single(const math::Ray& ray, RenderingContext& context) const;
    RT_FORCE_NOINLINE void TraceRay_Simd8(const math::Ray_Simd8& ray, RenderingContext& context, RayColor* outColors) const;


    RT_FORCE_NOINLINE void Traverse_Leaf_Single(const math::Ray& ray, const BVH::Node& node, HitPoint& outHitPoint) const;
    RT_FORCE_NOINLINE void Traverse_Leaf_Simd8(const math::Ray_Simd8& ray, const BVH::Node& node, HitPoint_Simd8& outHitPoint) const;

    /*
    RT_FORCE_NOINLINE void Traverse_Packet(const RayPacket& packet, RenderingContext& context, HitPoint_Packet& outHitPoints) const;

    // perform ray packet shading:
    // 1. apply calculated color to render target
    // 2. generate secondary rays
    RT_FORCE_NOINLINE void ShadePacket(const RayPacket& packet, const HitPoint_Packet& hitPoints, RenderingContext& context, Bitmap& renderTarget) const;
    */

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
