#pragma once

#include "../Core/Shapes/MeshShape.h"
#include "../Core/Material/Material.h"
#include "../Core/Utils/Bitmap.h"

#include <memory>
#include <vector>


namespace helpers {

using MaterialsMap = std::map<std::string, rt::MaterialPtr>;

rt::BitmapPtr LoadBitmapObject(const std::string& baseDir, const std::string& path);
rt::TexturePtr LoadTexture(const std::string& baseDir, const std::string& path);
rt::MeshShapePtr LoadMesh(const std::string& filePath, MaterialsMap& outMaterials, const float scale = 1.0f);
rt::MaterialPtr CreateDefaultMaterial(MaterialsMap& outMaterials);

} // namespace helpers