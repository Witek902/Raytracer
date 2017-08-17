#pragma once

#include "RayLib.h"
#include "MaterialLayer.h"

#include <string>


namespace rt {


class RAYLIB_API Material
{
public:
    Material();
    Material(const Material&) = default;
    Material(Material&&) = default;
    Material& operator = (const Material&) = default;
    Material& operator = (Material&&) = default;

    virtual ~Material() { }

    std::string debugName;

    math::Vector4 emissionColor;
    math::Vector4 diffuseColor;

    // TODO material layers
    // TODO PBR parameters
};

} // namespace rt
