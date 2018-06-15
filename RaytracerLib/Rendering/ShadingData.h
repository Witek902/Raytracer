#pragma once

#include "../Common.h"
#include "../Math/Vector4.h"


namespace rt {

class Material;

struct ShadingData
{
    const Material* material = nullptr;

    math::Vector4 position;

    math::Vector4 tangent;
    math::Vector4 bitangent;
    math::Vector4 normal;

    math::Vector4 texCoord;

    math::Vector4 LocalToWorld(const math::Vector4 localCoords) const;
    math::Vector4 WorldToLocal(const math::Vector4 worldCoords) const;
};


} // namespace rt
