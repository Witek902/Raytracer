#include "PCH.h"
#include "Material.h"
#include "BSDF.h"
#include "Mesh/Mesh.h"
#include "Rendering/ShadingData.h"
#include "Utils/Bitmap.h"
#include "Math/Random.h"
#include "Math/Utils.h"

namespace rt {

using namespace math;

DispersionParams::DispersionParams()
{
    // BK7 glass
    B[0] = 1.03961212f;
    B[1] = 0.231792344f;
    B[2] = 1.01046945f;
    C[0] = 6.00069867e-3f;
    C[1] = 2.00179144e-2f;
    C[2] = 1.03560653e+2f;
}

Material::Material(const char* debugName)
    : debugName(debugName)
{
}

Material::~Material() = default;
Material::Material(Material&&) = default;
Material& Material::operator = (Material&&) = default;

void Material::Compile()
{
    emissionColor = Vector4::Max(Vector4(), emissionColor);
    baseColor = Vector4::Max(Vector4(), Vector4::Min(VECTOR_ONE, baseColor));

    if (transparent)
    {
        mDiffuseBSDF = std::make_unique<TransparencyBSDF>(*this); // TODO
    }
    else
    {
        mDiffuseBSDF = std::make_unique<OrenNayarBSDF>(baseColor, roughness);
    }

    mSpecularBSDF = std::make_unique<CookTorranceBSDF>(roughness);
}

math::Vector4 Material::GetBaseColor(const math::Vector4 uv) const
{
    math::Vector4 color = baseColor;

    if (baseColorMap)
    {
        color *= baseColorMap->Sample(uv, SamplerDesc());
    }

    return color;
}

math::Vector4 Material::GetNormalVector(const math::Vector4 uv) const
{
    math::Vector4 normal(0.0f, 0.0f, 1.0f, 0.0f);

    if (normalMap)
    {
        SamplerDesc sampler;
        sampler.forceLinearSpace = true;

        normal = normalMap->Sample(uv, sampler);

        // scale from [0...1] to [-1...1]
        normal += normal;
        normal -= VECTOR_ONE;

        // reconstruct Z
        normal.z = sqrtf(1.0f - normal.x * normal.x - normal.y * normal.y);
    }

    return normal;
}

Float Material::GetRoughness(const math::Vector4 uv) const
{
    float value = roughness;

    if (roughnessMap)
    {
        value *= roughnessMap->Sample(uv, SamplerDesc()).x;
    }

    return value;
}

Float Material::GetMetalness(const math::Vector4 uv) const
{
    float value = metalness;

    if (metalnessMap)
    {
        value *= metalnessMap->Sample(uv, SamplerDesc()).x;
    }

    return value;
}

Bool Material::GetMaskValue(const math::Vector4 uv) const
{
    if (maskMap)
    {
        const float maskTreshold = 0.5f;
        return maskMap->Sample(uv, SamplerDesc()).x > maskTreshold;
    }

    return true;
}

Color Material::Shade(Wavelength& wavelength,
                      const math::Vector4& outgoingDirWorldSpace, math::Vector4& outIncomingDirWorldSpace,
                      const ShadingData& shadingData, math::Random& randomGenerator) const
{
    const Vector4 outgoingDirLocalSpace = shadingData.WorldToLocal(outgoingDirWorldSpace);
    const float NdotV = outgoingDirLocalSpace[2];

    const BSDF* bsdf = nullptr;
    math::Vector4 value; // TODO spectral color definitions

    const float metalnessValue = GetMetalness(shadingData.texCoord);

    if (randomGenerator.GetFloat() < metalnessValue)
    {
        value = GetBaseColor(shadingData.texCoord);
        value *= FresnelMetal(NdotV, IoR, K);
        bsdf = mSpecularBSDF.get();
    }
    else
    {
        bool totalInternalReflection = false;
        const float F = FresnelDielectric(NdotV, IoR, totalInternalReflection);
        const float specularWeight = F;

        if (randomGenerator.GetFloat() < specularWeight || totalInternalReflection) // glossy reflection
        {
            value = VECTOR_ONE;
            bsdf = mSpecularBSDF.get();
        }
        else // diffuse reflection / refraction
        {
            value = GetBaseColor(shadingData.texCoord);
            bsdf = mDiffuseBSDF.get();
        }
    }

    // BSDF sampling (in local space)
    Color weight;
    Vector4 incomingDirLocalSpace;
    bsdf->Sample(wavelength, outgoingDirLocalSpace, incomingDirLocalSpace, weight, randomGenerator);

    // convert incoming light direction back to world space
    outIncomingDirWorldSpace = shadingData.LocalToWorld(incomingDirLocalSpace);

    return weight * Color::SampleRGB(wavelength, value);
}

} // namespace rt
