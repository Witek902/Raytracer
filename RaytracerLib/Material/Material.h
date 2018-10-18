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

template<typename T>
struct MaterialParameter
{
    T baseValue;
    Bitmap* texture = nullptr;

    RT_FORCE_INLINE const math::Vector4 Sample(const math::Vector4 uv) const
    {
        T value = baseValue;

        if (texture)
        {
            // TODO
            value *= texture->Sample(uv, SamplerDesc());
        }

        return value;
    }
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

    // textures
    Bitmap* maskMap = nullptr;
    Bitmap* emissionColorMap = nullptr;
    Bitmap* baseColorMap = nullptr;
    Bitmap* normalMap = nullptr;
    Bitmap* roughnessMap = nullptr;
    Bitmap* metalnessMap = nullptr;

    // TODO material layers

    void Compile();

    const math::Vector4 GetNormalVector(const math::Vector4 uv) const;
    const math::Vector4 GetEmissionColor(const math::Vector4 uv) const;
    const math::Vector4 GetBaseColor(const math::Vector4 uv) const;
    Float GetRoughness(const math::Vector4 uv) const;
    Float GetMetalness(const math::Vector4 uv) const;
    Bool GetMaskValue(const math::Vector4 uv) const;

    // calculate amount of light reflected from incoming direction to outgoing direction
    const Color Evaluate(
        const Wavelength& wavelength,
        const ShadingData& shadingData,
        const math::Vector4& outgoingDirWorldSpace,
        const math::Vector4& incomingDirWorldSpace,
        Float* outPdfW = nullptr) const;

    // sample material's BSDFs
    const Color Sample(
        Wavelength& wavelength,
        const math::Vector4& outgoingDirWorldSpace,
        math::Vector4& outIncomingDirWorldSpace,
        const ShadingData& shadingData,
        math::Random& randomGenerator,
        Float* outPdfW = nullptr) const;

private:
    Material(const Material&) = delete;
    Material& operator = (const Material&) = delete;

    std::unique_ptr<BSDF> mDiffuseBSDF;
    std::unique_ptr<BSDF> mSpecularBSDF;
    std::unique_ptr<BSDF> mTransparencyBSDF;
};

} // namespace rt
