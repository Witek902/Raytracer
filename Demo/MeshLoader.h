#pragma once

#include "../Core/Mesh/Mesh.h"
#include "../Core/Material/Material.h"

#include <memory>
#include <vector>


namespace helpers {

using MaterialsMap = std::map<std::string, rt::MaterialPtr>;

rt::TexturePtr LoadTexture(const std::string& baseDir, const std::string& path);
rt::MeshPtr LoadMesh(const std::string& filePath, MaterialsMap& outMaterials, const float scale = 1.0f);
rt::MaterialPtr CreateDefaultMaterial(MaterialsMap& outMaterials);

} // namespace helpers