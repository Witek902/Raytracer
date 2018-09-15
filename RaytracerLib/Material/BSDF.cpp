#include "PCH.h"
#include "BSDF.h"
#include "Material.h"
#include "Math/Random.h"
#include "Math/Transcendental.h"
#include "Color/Color.h"


namespace rt {

using namespace math;

OrenNayarBSDF::OrenNayarBSDF()
    : mBaseColor(1.0f, 1.0f, 1.0f, 0.0f)
    , mRoughness(0.1f)
{}

OrenNayarBSDF::OrenNayarBSDF(const math::Vector4& baseColor, const float roughness)
    : mBaseColor(baseColor)
    , mRoughness(roughness)
{}

void OrenNayarBSDF::Sample(Wavelength& wavelength, const math::Vector4& outgoingDir, math::Vector4& outIncomingDir, Color& outWeight, math::Random& randomGenerator) const
{
    RT_UNUSED(wavelength);
    RT_UNUSED(outgoingDir);

    // generate random sine-weighted hemisphere vector
    {
        // TODO optimize sqrtf, sin and cos (use approximations)
        const Float2 u = randomGenerator.GetFloat2();
        const Vector4 t = Vector4::Sqrt4(Vector4(u.x, 1.0f - u.x, 0.0f, 0.0f));
        float theta = 2.0f * RT_PI * u.y;
        outIncomingDir = Vector4(t.x * Sin(theta), t.x * Cos(theta), t.y, 0.0f);
    }

    // TODO this is broken
    /*
    const float roughness2 = mRoughness * mRoughness;
    const float A = 1.0f - 0.5f * roughness2 / (0.33f + roughness2);
    const float B = 0.45f * roughness2 / (0.09f + roughness2);

    const float NdotI = incomingDir[2];
    const float NdotO = outgoingDir[2];
    const float IdotO = Max(0.0f, Vector4::Dot3(outgoingDir, incomingDir));

    const float s = IdotO + NdotI * NdotO;
    const float t = s < 0.0f ? Max(NdotI, NdotO) : 1.0f;

    return mBaseColor * ((A + B * s / t) / RT_PI);
    */

    outWeight = Color::One();
}

math::Vector4 OrenNayarBSDF::Evaluate(const math::Vector4& outgoingDir, const math::Vector4& incomingDir) const
{
    RT_UNUSED(outgoingDir);

    const float NdotL = incomingDir[2];
    return Vector4(NdotL);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CookTorranceBSDF::CookTorranceBSDF(const float roughness)
    : mRougness(roughness)
{}

float CookTorranceBSDF::NormalDistribution(const float NdotH) const
{
    // GGX distribution
    const float a = mRougness * mRougness;
    const float denomTerm = (a * a - 1.0f) * (NdotH * NdotH) + 1.0f;
    return (a * a) / (RT_PI * denomTerm * denomTerm);
}

void CookTorranceBSDF::Sample(Wavelength& wavelength, const math::Vector4& outgoingDir, math::Vector4& outIncomingDir, Color& outWeight, math::Random& randomGenerator) const
{
    RT_UNUSED(wavelength);

    // generate microfacet normal vector using GGX distribution function (Trowbridge-Reitz)
    const float a = mRougness * mRougness;
    const Float2 u = randomGenerator.GetFloat2();
    const float cosThetaSqr = (1.0f - u.x) / (1.0f + (a * a - 1.0f) * u.x);
    const float cosTheta = sqrtf(cosThetaSqr);
    const float sinTheta = sqrtf(1.0f - cosThetaSqr);
    const float phi = 2.0f * RT_PI * u.y;

    // microfacet normal (aka. half vector)
    const Vector4 m(sinTheta * cosf(phi), sinTheta * sinf(phi), cosTheta, 0.0f);

    outIncomingDir = -Vector4::Reflect3(outgoingDir, m);

    const float NdotH = m[2];
    const float NdotV = outgoingDir[2];
    const float NdotL = outIncomingDir[2];

    // clip the function to avoid division by zero
    if (Abs(NdotV) <= FLT_EPSILON || Abs(NdotL) <= FLT_EPSILON || NdotV * NdotL < 0.0f)
    {
        outWeight = Color::One();
        return;
    }

    // TODO this is wrong
    // Geometry term
    //const float G1 = 2.0f * NdotH * NdotV / VdotH;
    //const float G2 = 2.0f * NdotH * NdotL / VdotH;
    //const float G = Max(0.0f, Min(1.0f, Min(G1, G2)));
    const float G = 1.0f;

    outWeight = Color::One() * G;
}

math::Vector4 CookTorranceBSDF::Evaluate(const math::Vector4& outgoingDir, const math::Vector4& incomingDir) const
{
    const Vector4 half = (outgoingDir + incomingDir).FastNormalized3();
    const float NdotH = half[2];
    const float VdotH = Vector4::Dot3(half, outgoingDir);
    const float NdotV = outgoingDir[2];
    const float NdotL = incomingDir[2];

    // clip the function to avoid division by zero
    if (NdotV <= 0.0f || NdotL <= 0.0f)
    {
        return Vector4();
    }

    const float a2 = mRougness * mRougness;

    // Smith masking-shadowing term
    //const float G1 = NdotH * NdotV / VdotH;
    //const float G2 = NdotH * NdotL / VdotH;
    //const float G = Min(1.0f, Max(0.0f, 2.0f * Min(G1, G2)));
    const float denomA = NdotV * sqrtf(a2 + (1.0f - a2) * NdotL * NdotL);
    const float denomB = NdotL * sqrtf(a2 + (1.0f - a2) * NdotV * NdotV);
    const float G2 = 2.0f * NdotL * NdotV / (0.0001f + denomA + denomB);

    const float D = NormalDistribution(NdotH);

    return Vector4(D * G2 / (4.0f * NdotV * NdotL));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TransparencyBSDF::TransparencyBSDF(const Material& material)
    : material(material)
{
}

void TransparencyBSDF::Sample(Wavelength& wavelength, const math::Vector4& outgoingDir, math::Vector4& outIncomingDir, Color& outWeight, math::Random& randomGenerator) const
{
    RT_UNUSED(randomGenerator);

    float ior;

    if (material.isDispersive)
    {
        const float* B = material.dispersionParams.B;
        const float* C = material.dispersionParams.C;
        const float lambda = 1.0e+6f * Wavelength::Lower + wavelength.GetBase() * (Wavelength::Higher - Wavelength::Lower);
        const float lambda2 = lambda * lambda;
        ior = sqrtf(1.0f + B[0] * lambda / (lambda - C[0]) + B[1] * lambda / (lambda - C[1]) + B[2] * lambda / (lambda - C[2]));

        if (!wavelength.isSingle)
        {
            outWeight = Color::SingleWavelengthFallback();
            wavelength.isSingle = true;
        }
        else
        {
            outWeight = Color::One();
        }
    }
    else
    {
        ior = material.IoR;
        outWeight = Color::One();
    }

    outIncomingDir = Vector4::RefractZ(-outgoingDir, ior);
}

Vector4 TransparencyBSDF::Evaluate(const math::Vector4& outgoingDir, const math::Vector4& incomingDir) const
{
    RT_UNUSED(outgoingDir);
    RT_UNUSED(incomingDir);

    return Vector4();
}

} // namespace rt
