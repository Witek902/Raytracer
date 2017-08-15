#pragma once

#include "CpuRaytracing.h"
#include "../Scene.h"

#include "../Math/Vector4.h"
#include "../Math/Random.h"

// TODO remove
#include "../Math/Simd4Box.h"
#include "../Math/Simd8Box.h"

#include <vector>


namespace rt {

/**
 * Rendering scene.
 * Allows for placing objects (meshes, lights, etc.) and raytracing them.
 */
class RAYLIB_API CpuScene : public IScene
{
public:
    CpuScene();

    virtual MeshInstanceID CreateMeshInstance(const MeshInstance& data) override;
    virtual void DestroyMeshInstance(MeshInstanceID id) override;
    virtual void UpdateMeshInstance(MeshInstanceID id, const MeshInstance& data) override;

    // add a light to the scene
    virtual LightID CreateLightInstance(const LightInstance& data) override;

    // trace single (non-SIMD) ray
    RT_FORCE_NOINLINE math::Vector4 TraceRay_Single(const math::Ray& ray, RayTracingContext& context, Uint32 rayDepth) const;

private:
    RT_FORCE_NOINLINE void Raytrace_Single(const Camera& camera, Bitmap& renderTarget, const RaytracingParams& params);

    std::vector<MeshInstance> mMeshInstances;
    std::vector<LightInstance> mLightInstances;
    // TODO lights BVH
    // TODO meshes BVH
};

} // namespace rt
