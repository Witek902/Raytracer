#pragma once

#include "../RayLib.h"

#include "../Color/RayColor.h"
#include "../Traversal/HitPoint.h"
#include "../BVH/BVH.h"
#include "../Containers/DynArray.h"

namespace rt {

class ISceneObject;
class ITraceableSceneObject;
class ILight;
class LightSceneObject;
class ShapeSceneObject;
class DecalSceneObject;
struct RenderingContext;
struct HitPoint;
struct ShadingData;
struct IntersectionData;
struct SingleTraversalContext;
struct PacketTraversalContext;

using SceneObjectPtr = std::unique_ptr<ISceneObject>;
using LightPtr = std::unique_ptr<ILight>;

namespace math {
class Ray;
class Ray_Simd8;
} // namespace math

/**
 * Rendering scene.
 * Allows for placing objects (meshes, lights, etc.) and raytracing them.
 */
class RT_ALIGN(16) Scene : public Aligned<16>
{
public:
    RAYLIB_API Scene();
    RAYLIB_API ~Scene();
    RAYLIB_API Scene(Scene&&);
    RAYLIB_API Scene& operator = (Scene&&);

    //RAYLIB_API void AddLight(LightPtr object);
    RAYLIB_API void AddObject(SceneObjectPtr object);

    RAYLIB_API bool BuildBVH();

    RT_FORCE_INLINE const BVH& GetBVH() const { return mTraceableObjectsBVH; }
    RT_FORCE_INLINE const ITraceableSceneObject* GetHitObject(uint32 id) const { return mTraceableObjects[id]; }
    RT_FORCE_INLINE const DynArray<const LightSceneObject*>& GetLights() const { return mLights; }
    RT_FORCE_INLINE const DynArray<const LightSceneObject*>& GetGlobalLights() const { return mGlobalLights; }

    // traverse the scene, returns hit points
    RAYLIB_API void Traverse(const SingleTraversalContext& context) const;
    RAYLIB_API void Traverse(const PacketTraversalContext& context) const;

    // cast shadow ray
    bool Traverse_Shadow(const SingleTraversalContext& context) const;

    RAYLIB_API void EvaluateIntersection(const math::Ray& ray, const HitPoint& hitPoint, const float time, IntersectionData& outIntersectionData) const;

    void TraceRay_Simd8(const math::Ray_Simd8& ray, RenderingContext& context, RayColor* outColors) const;

    void Traverse_Leaf(const SingleTraversalContext& context, const uint32 objectID, const BVH::Node& node) const;
    void Traverse_Leaf(const PacketTraversalContext& context, const uint32 objectID, const BVH::Node& node, uint32 numActiveGroups) const;

    bool Traverse_Leaf_Shadow(const SingleTraversalContext& context, const BVH::Node& node) const;

    void EvaluateShadingData(ShadingData& shadingData, RenderingContext& context) const;

private:
    Scene(const Scene&) = delete;
    Scene& operator = (const Scene&) = delete;

    RT_FORCE_NOINLINE void Traverse_Object(const SingleTraversalContext& context, const uint32 objectID) const;
    RT_FORCE_NOINLINE bool Traverse_Object_Shadow(const SingleTraversalContext& context, const uint32 objectID) const;

    void EvaluateDecals(ShadingData& shadingData, RenderingContext& context) const;

    // keeps ownership
    DynArray<SceneObjectPtr> mAllObjects;

    DynArray<const LightSceneObject*> mLights;
    DynArray<const LightSceneObject*> mGlobalLights;

    DynArray<const ITraceableSceneObject*> mTraceableObjects;
    BVH mTraceableObjectsBVH;

    DynArray<const DecalSceneObject*> mDecals;
    BVH mDecalsBVH;
};

} // namespace rt
