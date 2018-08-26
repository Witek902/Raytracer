#include "PCH.h"
#include "Material.h"
#include "BSDF.h"
#include "Mesh/Mesh.h"
#include "Rendering/ShadingData.h"
#include "Utils/Bitmap.h"
#include "Math/Random.h"

#pragma optimize("", off)

namespace rt {

using namespace math;




float FresnelDielectric(float NdotV, float eta, bool& totalInternalReflection)
{
    if (NdotV > 0.0f)
    {
        eta = 1.0f / eta;
    }

    /*
    if (NdotV > 0.0f)
    {
        eta = 1.0f / eta;
    }
    else
    {
        NdotV = -NdotV;
    }

    float k = eta * eta * (1.0f - NdotV * NdotV);
    if (k > 1.0f)
    {
        //totalInternalReflection = true;
        return 1.0f; // TIR
    }

    const float g = sqrtf(1.0f - k);
    const float A = (NdotV - eta * g) / (NdotV + eta * g);
    const float B = (eta * NdotV - g) / (eta * NdotV + g);

    totalInternalReflection = false;
    return 0.5f * (A * A + B * B);
    */

    const float c = fabsf(NdotV);
    float g = eta * eta * (1.0f - NdotV * NdotV);
    if (g < 1.0f)
    {
        totalInternalReflection = false;
        g = sqrtf(1.0f - g);
        const float A = (g - c) / (g + c);
        const float B = (c * (g + c) - 1.0f) / (c * (g - c) + 1.0f);
        return 0.5f * A * A * (1.0f + B * B);
    }
    totalInternalReflection = true;return 1.0f;
}

float FresnelMetal(const float NdotV, const float eta, const float k)
{
    const float NdotV2 = NdotV * NdotV;
    const float a = eta * eta + k * k;
    const float b = a * NdotV2;
    const float rs = (b - (2.0f * eta * NdotV) + 1.0f) / (b + (2.0f * eta * NdotV) + 1.0f);
    const float rp = (a - (2.0f * eta * NdotV) + NdotV2) / (a + (2.0f * eta * NdotV) + NdotV2);
    return (rs + rp) * 0.5f;
}


Material::Material(const char* debugName)
    : debugName(debugName)
{
}

void Material::Compile()
{
    emissionColor = Vector4::Max(Vector4(), emissionColor);
    baseColor = Vector4::Max(Vector4(), Vector4::Min(VECTOR_ONE, baseColor));

    if (transparent)
    {
        mDiffuseBSDF = std::make_unique<TransparencyBSDF>(IoR); // TODO
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

math::Vector4 Material::Shade(const math::Vector4& outgoingDirWorldSpace, math::Vector4& outIncomingDirWorldSpace,
                              const ShadingData& shadingData, math::Random& randomGenerator) const
{
    const Vector4 outgoingDirLocalSpace = shadingData.WorldToLocal(outgoingDirWorldSpace);
    const float NdotV = outgoingDirLocalSpace[2];

    const BSDF* bsdf = nullptr;
    math::Vector4 value;

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
    Vector4 weight;
    Vector4 incomingDirLocalSpace;
    bsdf->Sample(outgoingDirLocalSpace, incomingDirLocalSpace, weight, randomGenerator);

    // convert incoming light direction back to world space
    outIncomingDirWorldSpace = shadingData.LocalToWorld(incomingDirLocalSpace);

    return value * weight;
}

} // namespace rt
