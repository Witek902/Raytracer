#pragma once

#include "RayLib.h"
#include "MaterialLayer.h"


namespace rt {


class RAYLIB_API Material
{
public:
    Material();
    virtual ~Material() { }

    math::Vector4 emissionColor;
    math::Vector4 diffuseColor;

    // TODO material layers
    // TODO PBR parameters
};

} // namespace rt
