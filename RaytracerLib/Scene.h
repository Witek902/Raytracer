#pragma once

#include "RayLib.h"
#include "Config.h"
#include "MeshInstance.h"
#include "LightInstance.h"


namespace rt {

class Bitmap;
class Camera;

namespace math {
class Ray;
} // namespace math

using LightID = Uint32;
using MeshInstanceID = Uint32;


struct RaytracingParams
{
    // maximum ray depth
    Uint32 maxRayDepth;

    // Antialiasing factor: 1.0 is most optimal
    // Setting to values above 1 will blur the image
    Float antiAliasingSpread;

    RaytracingParams()
        : maxRayDepth(12)
        , antiAliasingSpread(1.4f) // blur a little bit - real images are not perfectly sharp
    { }
};


/**
 * Global environment settings.
 */
struct SceneEnvironment
{
    math::Vector4 backgroundColor;
    math::Vector4 fogColor;
    float fogDensity;

    // TODO background texture

    SceneEnvironment()
        : backgroundColor(math::Vector4(1.0f, 1.0f, 1.0f))
        , fogColor(math::Vector4(1.0f, 1.0f, 1.0f))
        , fogDensity(0.01f)
    { }
};

/**
 * Rendering scene.
 * Allows for placing objects (meshes, lights, etc.) and raytracing them.
 */
class RAYLIB_API IScene
{
public:
    virtual ~IScene() { }

    // add a mesh to the scene
    virtual MeshInstanceID CreateMeshInstance(const MeshInstance& data) = 0;

    // remove mesh from the scene
    virtual void DestroyMeshInstance(MeshInstanceID id) = 0;

    // modify existing mesh instance
    virtual void UpdateMeshInstance(MeshInstanceID id, const MeshInstance& data) = 0;

    // add a light to the scene
    virtual LightID CreateLightInstance(const LightInstance& data) = 0;
};

} // namespace rt
