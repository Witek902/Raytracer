#pragma once

#include "../Instance.h"

namespace rt {

/**
 * CPU raytracer instance.
 */
class CpuInstance : public Instance
{
public:
    virtual std::unique_ptr<Material> CreateMaterial() override;
    virtual std::unique_ptr<IMesh> CreateMesh() override;
    virtual std::unique_ptr<IScene> CreateScene() override;
    virtual std::unique_ptr<IViewport> CreateViewport() override;
};

} // namespace rt
