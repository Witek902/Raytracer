#pragma once

#include "RayLib.h"

#include <memory>


namespace rt {

class IMesh;
class IScene;
class IViewport;
class Material;

/**
* Raytracer instance.
*/
class RAYLIB_API Instance
{
public:
    virtual ~Instance() { }

    virtual std::unique_ptr<Material> CreateMaterial() = 0;
    virtual std::unique_ptr<IMesh> CreateMesh() = 0;
    virtual std::unique_ptr<IScene> CreateScene() = 0;
    virtual std::unique_ptr<IViewport> CreateViewport() = 0;

    static std::unique_ptr<Instance> CreateCpuInstance();
    static std::unique_ptr<Instance> CreateGpuInstance();
};

} // namespace rt
