#include "PCH.h"
#include "CpuInstance.h"
#include "CpuScene.h"
#include "CpuMesh.h"
#include "CpuViewport.h"
#include "../Material.h"


namespace rt {

std::unique_ptr<Material> CpuInstance::CreateMaterial()
{
    // no CPU specialization needed
    return std::make_unique<Material>();
}

std::unique_ptr<IMesh> CpuInstance::CreateMesh()
{
    return std::make_unique<CpuMesh>();
}

std::unique_ptr<IScene> CpuInstance::CreateScene()
{
    return std::make_unique<CpuScene>();
}

std::unique_ptr<IViewport> CpuInstance::CreateViewport()
{
    return std::make_unique<CpuViewport>();
}

} // namespace rt