#include "PCH.h"
#include "Material.h"
#include "Mesh/Mesh.h"
#include "Utils/Bitmap.h"


namespace rt {

using namespace math;


// Oren-Nayar BRDF - diffuse term
float OrenNayar(const Vector4& normal, const Vector4& in, const Vector4& out, float roughness)
{
    const float roughness2 = roughness * roughness;
    const float A = 1.0f - 0.5f * roughness2 / (0.33f + roughness2);
    const float B = 0.45f * roughness2 / (0.09f + roughness2);

    const float NdotI = Max(0.0f, Vector4::Dot3(normal, in));
    const float NdotO = Max(0.0f, Vector4::Dot3(normal, out));
    const float IdotO = Max(0.0f, Vector4::Dot3(out, in));

    const float s = IdotO + NdotI * NdotO;
    const float t = s < 0.0f ? Max(NdotI, NdotO) : 1.0f;

    return A + B * s / t;
}

// Cook-Torrance BRDF - specular term
float CookTorrance(const Vector4& normal, const Vector4& in, const Vector4& out, float roughness)
{
    const float R0 = 0.1f; // TODO index of refraction

    // Correct the input and compute aliases
    const Vector4 viewDir = -in;
    const Vector4 lightDir = out;
    const Vector4 half = (viewDir + lightDir).Normalized3();
    float NormalDotHalf = Vector4::Dot3(normal, half);
    float ViewDotHalf = Vector4::Dot3(half, viewDir);
    float NormalDotView = Vector4::Dot3(normal, viewDir);
    float NormalDotLight = Vector4::Dot3(normal, lightDir);

    // Compute the geometric term
    float G1 = (2.0f * NormalDotHalf * NormalDotView) / ViewDotHalf;
    float G2 = (2.0f * NormalDotHalf * NormalDotLight) / ViewDotHalf;
    float G = Min(1.0f, Max(0.0f, Min(G1, G2)));

    // Compute the Fresnel term
    float f = 1.0f - NormalDotView;
    float f2 = f * f;
    float f4 = f2 * f2;
    float f5 = f * f4;
    float F = R0 + (1.0f - R0) * f5;

    // Compute the roughness term
    float R_2 = roughness * roughness;
    float NDotH_2 = NormalDotHalf * NormalDotHalf;
    float A = 1.0f / (0.005f + 4.0f * R_2 * NDotH_2 * NDotH_2);
    float B = expf(-(1.0f - NDotH_2) / (0.005f + R_2 * NDotH_2));
    float R = A * B;

    // Compute the final term
    return (G * F * R) / (0.005f + NormalDotLight * NormalDotView);
}

Material::Material()
{ }

math::Vector4 Material::GetBaseColor(const math::Vector4 uv) const
{
    math::Vector4 color = baseColor;

    if (baseColorMap)
    {
        color *= baseColorMap->Sample(uv, SamplerDesc());
    }

    return color;
}

bool Material::GenerateSecondaryRay(const math::Vector4& incomingDir, const ShadingData& shadingData, math::Random& randomGenerator,
                                    math::Ray& outRay, math::Vector4& outRayFactor) const
{
    if (baseColor.IsZero())
    {
        return false;
    }

    // generate secondary ray
    const Vector4 localDir = randomGenerator.GetHemishpereCos();
    const Vector4 origin = shadingData.position + shadingData.normal * 0.001f;
    const Vector4 globalDir = shadingData.tangent * localDir[0] + shadingData.binormal * localDir[1] + shadingData.normal * localDir[2];
    outRay = Ray(origin, globalDir);

    // TODO texturing

    const float diffuse = 1.0f; // OrenNayar(shadingData.normal, incomingDir, outRay.dir, roughness);
    const float specular = CookTorrance(shadingData.normal, incomingDir, outRay.dir, roughness);

    outRayFactor = Vector4::Splat(Lerp(diffuse, specular, metalness));

    return true;
}

} // namespace rt
