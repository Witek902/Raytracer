#pragma once

#include "../RayLib.h"

#include "MeshInstance.h"

#include "../Traversal/IntersectionData.h"


#include <vector>


namespace rt {

class Camera;
struct RenderingContext;

namespace math {
class Ray;
} // namespace math

using LightID = Uint32;
using MeshInstanceID = Uint32;


/**
 * Global environment settings.
 */
struct SceneEnvironment
{
    math::Vector4 backgroundColor;

    // describes how much of background light will "leak" to a ray
    float fogDensity;

    // TODO background texture

    SceneEnvironment()
        : backgroundColor(math::Vector4(0.3f, 0.4f, 0.5f))
        , fogDensity(0.01f)
    { }
};

/**
 * Rendering scene.
 * Allows for placing objects (meshes, lights, etc.) and raytracing them.
 */
class RAYLIB_API Scene
{
public:
    Scene();

    MeshInstanceID CreateMeshInstance(const MeshInstance& data);
    void DestroyMeshInstance(MeshInstanceID id);
    void UpdateMeshInstance(MeshInstanceID id, const MeshInstance& data);

    // trace single (non-SIMD) ray
    RT_FORCE_NOINLINE math::Vector4 TraceRay_Single(const math::Ray& ray, RenderingContext& context) const;

    // traverse a packet and return intersection data
    RT_FORCE_NOINLINE void Traverse_Packet(const RayPacket& packet, RenderingContext& context, RayPacketIntersectionData& outIntersectionData) const;

private:
    SceneEnvironment mEnvironment;

    std::vector<MeshInstance> mMeshInstances;
};

} // namespace rt
