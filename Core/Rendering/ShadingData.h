#pragma once

#include "../Common.h"
#include "../Color/RayColor.h"
#include "../Math/Matrix4.h"

namespace rt {

class Material;

struct SampledMaterialParameters
{
    RayColor baseColor;
    float roughness;
    float metalness;
    float IoR;
};

struct ShadingData
{
    // geometry data
    math::Matrix4 frame;
    math::Vector4 texCoord;

    const Material* material = nullptr;

    // incoming ray data
    math::Vector4 outgoingDirWorldSpace;

    SampledMaterialParameters materialParams;

    const math::Vector4 LocalToWorld(const math::Vector4& localCoords) const;
    const math::Vector4 WorldToLocal(const math::Vector4& worldCoords) const;

    float CosTheta(const math::Vector4& dir) const;
};

struct PackedShadingData
{
    const Material* material = nullptr; // TODO can be turned into Uint32

    math::Float3 position;
    math::Float3 normal;    // TODO can be packed more
    math::Float3 tangent;   // TODO can be packed more
    math::Float2 texCoord;

    math::Float3 outgoingDirWorldSpace;

    SampledMaterialParameters materialParams; // TODO can be packed more
};

void PackShadingData(PackedShadingData& outPacked, const ShadingData& input);
void UnpackShadingData(ShadingData& outUnpacked, const PackedShadingData& input);

} // namespace rt
