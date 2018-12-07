#pragma once

#include "../Core/Scene/Scene.h"

namespace helpers {

bool LoadScene(const std::string& path, rt::Scene& scene, rt::Camera& camera);

} // namespace helpers