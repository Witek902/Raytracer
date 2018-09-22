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

OrenNayarBSDF::OrenNayarBSDF(const Vector4& baseColor, const float roughness)
    : mBaseColor(baseColor)
    , mRoughness(roughness)
{}

void OrenNayarBSDF::Sample(Wavelength& wavelength, const Vector4& outgoingDir, Vector4& outIncomingDir, Color& outWeight, Random& randomGenerator) const
{
    RT_UNUSED(wavelength);

    // generate random sine-weighted hemisphere vector
    {
        // TODO optimize sqrtf, sin and cos (use approximations)
        const Float2 u = randomGenerator.GetFloat2();
        const Vector4 t = Vector4::Sqrt4(Vector4(u.x, 1.0f - u.x, 0.0f, 0.0f));
        float theta = 2.0f * RT_PI * u.y;
        outIncomingDir = Vector4(t.x * Sin(theta), t.x * Cos(theta), t.y, 0.0f);
    }

    const float NdotV = outgoingDir.z;
    const float NdotL = outIncomingDir.z;

    if (NdotV > 0.0f || NdotL > 0.0f)
    {
        const float LdotV = Max(0.0f, Vector4::Dot3(outgoingDir, -outIncomingDir));

        const float s2 = mRoughness * mRoughness;
        const float A = 1.0f - 0.50f * s2 / (0.33f + s2);
        const float B = 0.45f * s2 / (0.09f + s2);

        // based on http://mimosa-pudica.net/improved-oren-nayar.html
        const float s = LdotV - NdotL * NdotV;
        const float stinv = s > 0.0f ? s / Max(NdotL, NdotV) : 0.0f;

        outWeight = Color::One() * Max(A + B * stinv, 0.0f);
    }
    else
    {
        outWeight = Color();
    }
}

Vector4 OrenNayarBSDF::Evaluate(const Vector4& outgoingDir, const Vector4& incomingDir) const
{
    RT_UNUSED(outgoingDir);

    const float NdotV = outgoingDir.z;
    const float NdotL = -incomingDir.z;

    if (NdotV > 0.0f || NdotL > 0.0f)
    {
        const float LdotV = Max(0.0f, Vector4::Dot3(outgoingDir, -incomingDir));

        const float s2 = mRoughness * mRoughness;
        const float A = 1.0f - 0.50f * s2 / (0.33f + s2);
        const float B =        0.45f * s2 / (0.09f + s2);

        // based on http://mimosa-pudica.net/improved-oren-nayar.html
        const float s = LdotV - NdotL * NdotV;
        const float stinv = s > 0.0f ? s / Max(NdotL, NdotV) : 0.0f;

        return Vector4(Max(NdotL * (A + B * stinv), 0.0f));
    }

    return Vector4();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CookTorranceBSDF::CookTorranceBSDF(const float roughness)
    : mRougness(roughness)
{}

static float NormalDistribution_GGX(const float NdotH, const float roughness)
{
    // GGX distribution
    const float a = roughness * roughness;
    const float a2 = a * a;
    const float denomTerm = (a2 - 1.0f) * (NdotH * NdotH) + 1.0f;
    return a2 / (RT_PI * denomTerm * denomTerm);
}

void CookTorranceBSDF::Sample(Wavelength& wavelength, const Vector4& outgoingDir, Vector4& outIncomingDir, Color& outWeight, Random& randomGenerator) const
{
    RT_UNUSED(wavelength);

    const float a = mRougness * mRougness;
    const float a2 = a * a;

    // generate microfacet normal vector using GGX distribution function (Trowbridge-Reitz)
    const Float2 u = randomGenerator.GetFloat2();
    const float cosThetaSqr = (1.0f - u.x) / (1.0f + (a2 - 1.0f) * u.x);
    const float cosTheta = sqrtf(cosThetaSqr);
    const float sinTheta = sqrtf(1.0f - cosThetaSqr);
    const float phi = 2.0f * RT_PI * u.y;

    // microfacet normal (aka. half vector)
    const Vector4 m(sinTheta * cosf(phi), sinTheta * sinf(phi), cosTheta, 0.0f);

    outIncomingDir = -Vector4::Reflect3(outgoingDir, m);

    const float NdotV = outgoingDir.z;
    const float NdotL = outIncomingDir.z;

    // clip the function to avoid division by zero
    if (NdotV < FLT_EPSILON || NdotL < FLT_EPSILON)
    {
        outWeight = Color();
        return;
    }

    // GGX masking-shadowing term
    /*
    const float G1 = 2.0f * NdotV / (NdotV + sqrtf(a2 + (1.0f - a2) * NdotV * NdotV));
    const float G2 = 2.0f * NdotL / (NdotL + sqrtf(a2 + (1.0f - a2) * NdotL * NdotL));
    const float G = G1 * G2;
    */
    const float G = 1.0f;

    outWeight = Color::One() * G;
}

Vector4 CookTorranceBSDF::Evaluate(const Vector4& outgoingDir, const Vector4& incomingDir) const
{
    const Vector4 half = (outgoingDir - incomingDir).FastNormalized3();
    const float NdotH = half.z;
    // const float VdotH = Vector4::Dot3(half, outgoingDir);
    const float NdotV = outgoingDir.z;
    const float NdotL = -incomingDir.z;

    // clip the function
    if (NdotV <= 0.0f || NdotL <= 0.0f)
    {
        return Vector4();
    }

    const float a = mRougness * mRougness;
    const float a2 = a * a;

    // GGX masking-shadowing term
    const float G1 = NdotV / (NdotV + sqrtf(a2 + (1.0f - a2) * NdotV * NdotV));
    const float G2 = NdotL / (NdotL + sqrtf(a2 + (1.0f - a2) * NdotL * NdotL));
    const float G = G1 * G2;

    const float D = NormalDistribution_GGX(NdotH, mRougness);

    return Vector4(D * G / (NdotV));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TransparencyBSDF::TransparencyBSDF(const Material& material)
    : material(material)
{
}

void TransparencyBSDF::Sample(Wavelength& wavelength, const Vector4& outgoingDir, Vector4& outIncomingDir, Color& outWeight, Random& randomGenerator) const
{
    RT_UNUSED(randomGenerator);

    float ior;

    if (material.isDispersive)
    {
        const float* B = material.dispersionParams.B;
        const float* C = material.dispersionParams.C;
        const float lambda = 1.0e+6f * Wavelength::Lower + wavelength.GetBase() * (Wavelength::Higher - Wavelength::Lower);
        const float lambda2 = lambda * lambda;
        ior = sqrtf(1.0f + B[0] * lambda2 / (lambda2 - C[0]) + B[1] * lambda2 / (lambda2 - C[1]) + B[2] * lambda2 / (lambda2 - C[2]));

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

Vector4 TransparencyBSDF::Evaluate(const Vector4& outgoingDir, const Vector4& incomingDir) const
{
    RT_UNUSED(outgoingDir);
    RT_UNUSED(incomingDir);

    return Vector4();
}

} // namespace rt
