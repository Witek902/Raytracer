#pragma once

#include "../RayLib.h"

#include "BSDF.h"
#include "MaterialLayer.h"

#include "../Math/Ray.h"

#include <string>


namespace rt {

namespace math
{
class Random;
}

struct ShadingData;
class Bitmap;
class BSDF;

// simple PBR material
class RAYLIB_API Material
{
public:
    Material(const char* debugName = "<unnamed>");
    Material(Material&&) = default;
    Material& operator = (Material&&) = default;

    std::string debugName;

    // light emitted by the material itself
    // useful for lamps, etc
    math::Vector4 emissionColor;

    // a.k.a. albedo
    // for metals this is specular/reflection color
    // for dielectrics this is diffuse color
    math::Vector4 baseColor = math::Vector4(0.6f, 0.6f, 0.6f, 0.0f);

    // amount of reflection
    float specular = 1.0f;

    // 0.0 - smooth, perfect mirror
    // 1.0 - rough, maximum dispersion
    float roughness = 0.1f;

    // index of refraction (real and imaginary parts)
    float IoR;
    float K;

    // selects between dielectric/metal models
    // TODO make it float that blends the models smoothly
    bool metal;

    bool transparent;

    // textures
    Bitmap* emissionColorMap = nullptr;
    Bitmap* baseColorMap = nullptr;
    Bitmap* normalMap = nullptr;
    // TODO metal/roughness map

    // TODO material layers

    void Compile();

    math::Vector4 GetBaseColor(const math::Vector4 uv) const;
    math::Vector4 GetNormalVector(const math::Vector4 uv) const;

    // Shade a ray and generate secondary ray
    // TODO wavelength
    math::Vector4 Shade(const math::Vector4& outgoingDirWorldSpace, math::Vector4& outIncomingDirWorldSpace,
                        const ShadingData& shadingData, math::Random& randomGenerator) const;

private:

    Material(const Material&) = delete;
    Material& operator = (const Material&) = delete;

    std::unique_ptr<BSDF> mDiffuseBSDF;
    std::unique_ptr<BSDF> mSpecularBSDF;
};

} // namespace rt
