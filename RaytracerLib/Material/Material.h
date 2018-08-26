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
class RAYLIB_API RT_ALIGN(16) Material : public Aligned<16>
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
    math::Vector4 baseColor = math::Vector4(0.7f, 0.7f, 0.7f, 0.0f);

    // 0.0 - smooth, perfect mirror
    // 1.0 - rough, maximum dispersion
    float roughness = 0.1f;

    // index of refraction (real and imaginary parts)
    float IoR = 1.5f;
    float K = 4.0f;

    // blends between dielectric/metal models
    float metalness = 0.0f;

    bool transparent = false;

    // textures
    Bitmap* maskMap = nullptr;
    Bitmap* emissionColorMap = nullptr;
    Bitmap* baseColorMap = nullptr;
    Bitmap* normalMap = nullptr;
    Bitmap* roughnessMap = nullptr;
    Bitmap* metalnessMap = nullptr;

    // TODO material layers

    void Compile();

    math::Vector4 GetNormalVector(const math::Vector4 uv) const;
    math::Vector4 GetBaseColor(const math::Vector4 uv) const;
    Float GetRoughness(const math::Vector4 uv) const;
    Float GetMetalness(const math::Vector4 uv) const;
    Bool GetMaskValue(const math::Vector4 uv) const;

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
