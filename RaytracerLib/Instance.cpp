#include "PCH.h"
#include "Instance.h"
#include "CPU/CpuInstance.h"


namespace rt {

std::unique_ptr<Instance> Instance::CreateCpuInstance()
{
    return std::make_unique<CpuInstance>();
}

std::unique_ptr<Instance> Instance::CreateGpuInstance()
{
    // TODO
    return nullptr;
}

} // namespace rt