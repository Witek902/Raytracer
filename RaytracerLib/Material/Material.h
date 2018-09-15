#pragma once

#include "../RayLib.h"
#include "../Utils/AlignmentAllocator.h"
#include "../Color/Color.h"
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

// coefficients of Sellmeier dispersion equation
struct RAYLIB_API DispersionParams
{
    float B[3];
    float C[3];

    DispersionParams();
};

// simple PBR material
class RAYLIB_API RT_ALIGN(16) Material : public Aligned<16>
{
public:
    Material(const char* debugName = "<unnamed>");
    ~Material();
    Material(Material&&);
    Material& operator = (Material&&);

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
    float IoR = 1.5f; // NOTE: not used when material is dispersive
    float K = 4.0f;

    // chromatic dispersion parameters (used only if 'isDispersive' is enabled)
    DispersionParams dispersionParams;

    // blends between dielectric/metal models
    float metalness = 0.0f;

    // When enabled, index of refraction depends on wavelength according to Sellmeier equation
    bool isDispersive = false;

    bool transparent = false;

    // if set to true, there won't be reflected ray generated
    bool light = false;

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
    Color Shade(Wavelength& wavelength,
                const math::Vector4& outgoingDirWorldSpace, math::Vector4& outIncomingDirWorldSpace,
                const ShadingData& shadingData, math::Random& randomGenerator) const;

private:
    Material(const Material&) = delete;
    Material& operator = (const Material&) = delete;

    std::unique_ptr<BSDF> mDiffuseBSDF;
    std::unique_ptr<BSDF> mSpecularBSDF;
};

} // namespace rt
