#pragma once

#include "../RayLib.h"

#include "MaterialLayer.h"

#include "../Math/Random.h"
#include "../Math/Ray.h"

#include <string>


namespace rt {

struct ShadingData;
class Bitmap;

// simple PBR material
class RAYLIB_API Material
{
public:
    Material();
    Material(const Material&) = default;
    Material(Material&&) = default;
    Material& operator = (const Material&) = default;
    Material& operator = (Material&&) = default;

    std::string debugName;

    // light emitted by the material itself
    // useful for lamps, etc
    math::Vector4 emissionColor;

    // a.k.a. albedo
    // for metals this is specular/reflection color
    // for dielectrics this is diffuse color
    math::Vector4 baseColor = math::Vector4(0.6f, 0.6f, 0.6f);

    // only applicable for dielectric materials
    // NOTE: this should be set to one in most cases
    math::Vector4 specularColor = math::VECTOR_ONE;

    // 0.0 - smooth, perfect mirror
    // 1.0 - rough, maximum dispersion
    float roughness = 0.1f;

    // mixing factor between dielectric and metallic models
    float metalness = 0.1f;

    // 1.0 - transparent material
    // 0.0 - opaque material
    float opacity = 0.0f;

    // index of refraction for Fresnel term calculation
    float indexOfRefraction = 1.5f;

    // textures
    Bitmap* emissionColorMap = nullptr;
    Bitmap* baseColorMap = nullptr;
    Bitmap* specularColorMap = nullptr;

    // TODO material layers

    // sample base color
    math::Vector4 GetBaseColor(const math::Vector4 uv) const;

    RT_FORCE_NOINLINE
    bool GenerateSecondaryRay(const math::Vector4& incomingDir, const ShadingData& shadingData, math::Random& randomGenerator,
                              math::Ray& outRay, math::Vector4& outRayFactor) const;
};

} // namespace rt
