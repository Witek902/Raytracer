#include "PCH.h"
#include "Material.h"
#include "BSDF.h"
#include "Mesh/Mesh.h"
#include "Utils/Bitmap.h"
#include "Math/Random.h"


namespace rt {

using namespace math;

float FresnelDielectric(const float NdotV, const float eta)
{
    float c = fabsf(NdotV);
    float g = eta * eta - 1.0f + c * c;
    if (g > 0.0f)
    {
        g = sqrtf(g);
        const float A = (g - c) / (g + c);
        const float B = (c * (g + c) - 1.0f) / (c * (g - c) + 1.0f);
        return 0.5f * A * A * (1.0f + B * B);
    }
    return 1.0f;
}

float FresnelMetal(const float NdotV, const float eta, const float k)
{
    float NdotV2 = NdotV * NdotV;
    float a = eta * eta + k * k;
    float b = a * NdotV2;
    float rs = (b - (2.0f * eta * NdotV) + 1.0f) / (b + (2.0f * eta * NdotV) + 1.0f);
    float rp = (a - (2.0f * eta * NdotV) + NdotV2) / (a + (2.0f * eta * NdotV) + NdotV2);
    return (rs + rp) * 0.5f;
}


Material::Material(const char* debugName)
    : debugName(debugName)
    , metal(false)
    , IoR(1.5f)
    , K(4.0f)
{
}

void Material::Compile()
{
    emissionColor = Vector4::Max(Vector4(), emissionColor);
    baseColor = Vector4::Max(Vector4(), Vector4::Min(VECTOR_ONE, baseColor));

    mDiffuseBSDF = std::make_unique<OrenNayarBSDF>(baseColor, roughness);
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

float Material::GetSpecularValue(const math::Vector4 uv) const
{
    float value = specular;

    if (specularMap)
    {
        value *= specularMap->Sample(uv, SamplerDesc())[0];
    }

    return value;
}

math::Vector4 Material::Shade(const math::Vector4& outgoingDirWorldSpace, math::Vector4& outIncomingDirWorldSpace,
                              const ShadingData& shadingData, math::Random& randomGenerator) const
{
    const Vector4 outgoingDirLocalSpace = shadingData.WorldToLocal(outgoingDirWorldSpace);
    const float NdotV = outgoingDirLocalSpace[2];

    const BSDF* bsdf = nullptr;
    math::Vector4 value;

    if (metal)
    {
        value = GetBaseColor(shadingData.texCoord);
        value *= FresnelMetal(NdotV, IoR, K);
        bsdf = mSpecularBSDF.get();
    }
    else
    {
        const float F = FresnelDielectric(NdotV, IoR);
        const float specularWeight = F * GetSpecularValue(shadingData.texCoord);

        if (randomGenerator.GetFloat() < specularWeight) // glossy reflection
        {
            value = VECTOR_ONE;
            bsdf = mSpecularBSDF.get();
        }
        else // diffuse reflection / refraction
        {
            value = GetBaseColor(shadingData.texCoord);
            bsdf = mDiffuseBSDF.get();

            // TODO refraction
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
