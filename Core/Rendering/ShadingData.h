#pragma once

#include "../Common.h"
#include "../Color/RayColor.h"
#include "../Math/Matrix4.h"

namespace rt {

class Material;

struct SampledMaterialParameters
{
    RayColor baseColor;
    Float roughness;
    Float metalness;
    Float IoR;
};

struct ShadingData
{
    // geometry data
    math::Matrix4 frame;
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
