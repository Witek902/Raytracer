#pragma once

#include "../RaytracerLib/Mesh.h"
#include "../RaytracerLib/Material.h"
#include "../RaytracerLib/Instance.h"

#include <memory>
#include <vector>


namespace helpers {

using MaterialsList = std::vector<std::unique_ptr<rt::Material>>;

std::unique_ptr<rt::IMesh> LoadMesh(const std::string& filePath, rt::Instance& raytracerInstance, MaterialsList& outMaterials, const Float scale = 1.0f);
std::unique_ptr<rt::IMesh> CreatePlaneMesh(rt::Instance& raytracerInstance, MaterialsList& outMaterials, const Float scale = 1.0f);

} // namespace helpers