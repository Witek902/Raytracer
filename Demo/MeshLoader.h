#pragma once

#include "../Core/Mesh/Mesh.h"
#include "../Core/Material/Material.h"

#include <memory>
#include <vector>


namespace helpers {

using MaterialsList = std::vector<rt::MaterialPtr>;

rt::BitmapPtr LoadTexture(const std::string& baseDir, const std::string& path);
rt::MeshPtr LoadMesh(const std::string& filePath, MaterialsList& outMaterials, const Float scale = 1.0f);
rt::MaterialPtr CreateDefaultMaterial(MaterialsList& outMaterials);
rt::MeshPtr CreatePlane(MaterialsList& outMaterials, const Float size, const Float textureScale = 1.0f);

} // namespace helpers