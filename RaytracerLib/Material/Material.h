#pragma once

#include "BSDF/BSDF.h"
#include "../RayLib.h"
#include "../Utils/AlignmentAllocator.h"
#include "../Color/Color.h"
#include "../Math/Ray.h"
#include "../Utils/Bitmap.h" // TODO remove

#include <string>

namespace rt {

namespace math
{
class Random;
}

struct ShadingData;
class Bitmap;

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
    BitmapPtr texture = nullptr;

    MaterialParameter() = default;

    RT_FORCE_INLINE MaterialParameter(const T baseValue)
        : baseValue(baseValue)
    {}

    RT_FORCE_INLINE const T Evaluate(const math::Vector4 uv) const
    {
        T value = baseValue;

        if (texture)
        {
            value = static_cast<T>(value * texture->Sample(uv, SamplerDesc()));
        }

        return value;
    };
};

class Material;
using MaterialPtr = std::shared_ptr<rt::Material>;

// simple PBR material
class RAYLIB_API RT_ALIGN(16) Material : public Aligned<16>
{
public:
    Material(const char* debugName = "<unnamed>");
    ~Material();
    Material(Material&&);
    Material& operator = (Material&&);

    static MaterialPtr Create();

    static const Material* GetDefaultMaterial();

    std::string debugName;

    // light emitted by the material itself
    // useful for lamps, etc
    MaterialParameter<math::Vector4> emission = math::Vector4::Zero();

    // a.k.a. albedo
    // for metals this is specular/reflection color
    // for dielectrics this is diffuse color
    MaterialParameter<math::Vector4> baseColor = math::Vector4(0.7f, 0.7f, 0.7f, 0.0f);

    // 0.0 - smooth, perfect mirror
    // 1.0 - rough, maximum dispersion
    MaterialParameter<Float> roughness = 0.1f;

    // blends between dielectric/metal models
    MaterialParameter<Float> metalness = 0.0f;

    // normal map lerp value
    float normalMapStrength = 1.0f;

    // index of refraction (real and imaginary parts)
    float IoR = 1.5f; // NOTE: not used when material is dispersive
    float K = 4.0f;

    // chromatic dispersion parameters (used only if 'isDispersive' is enabled)
    DispersionParams dispersionParams;

    // When enabled, index of refraction depends on wavelength according to Sellmeier equation
    bool isDispersive = false;

    bool transparent = false;

    // textures
    BitmapPtr maskMap = nullptr;
    BitmapPtr normalMap = nullptr;

    // TODO material layers

    void Compile();

    const math::Vector4 GetNormalVector(const math::Vector4 uv) const;
    Bool GetMaskValue(const math::Vector4 uv) const;

    void EvaluateShadingData(const Wavelength& wavelength, ShadingData& shadingData) const;

    // sample material's BSDFs
    const Color Sample(
        Wavelength& wavelength,
        math::Vector4& outIncomingDirWorldSpace,
        const ShadingData& shadingData,
        math::Random& randomGenerator,
        Float& outPdfW,
        BSDF::EventType& outSampledEvent) const;

    // calculate amount of light reflected from incoming direction to outgoing direction
    const Color Evaluate(
        const Wavelength& wavelength,
        const ShadingData& shadingData,
        const math::Vector4& incomingDirWorldSpace,
        Float* outPdfW = nullptr) const;

private:
    Material(const Material&) = delete;
    Material& operator = (const Material&) = delete;

    std::unique_ptr<BSDF> mDiffuseBSDF;
    std::unique_ptr<BSDF> mSpecularBSDF;
    std::unique_ptr<BSDF> mTransparencyBSDF;
};

} // namespace rt
