#pragma once

#include "../Common.h"
#include "../Color/Color.h"

namespace rt {

class Material;

struct SampledMaterialParameters
{
    Color baseColor;
    Float roughness;
    Float metalness;
    Float IoR;
};

struct ShadingData
{
    // geometry data
    math::Vector4 position;
    math::Vector4 tangent;
    math::Vector4 bitangent;
    math::Vector4 normal;
    math::Vector4 texCoord;

    const Material* material = nullptr;

    // incoming ray data
    math::Vector4 outgoingDirLocalSpace;
    math::Vector4 outgoingDirWorldSpace;

    SampledMaterialParameters materialParams;

    RT_FORCE_NOINLINE const math::Vector4 LocalToWorld(const math::Vector4 localCoords) const;
    RT_FORCE_NOINLINE const math::Vector4 WorldToLocal(const math::Vector4 worldCoords) const;
};


} // namespace rt
