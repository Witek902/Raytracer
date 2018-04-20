#pragma once

#include "../RaytracerLib/Mesh/Mesh.h"
#include "../RaytracerLib/Material/Material.h"

#include <memory>
#include <vector>


namespace helpers {

using MaterialsList = std::vector<std::unique_ptr<rt::Material>>;

std::unique_ptr<rt::Mesh> LoadMesh(const std::string& filePath, MaterialsList& outMaterials, const Float scale = 1.0f);
std::unique_ptr<rt::Mesh> CreatePlaneMesh(MaterialsList& outMaterials, const Float scale = 1.0f);

} // namespace helpers