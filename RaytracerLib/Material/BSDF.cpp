#include "PCH.h"
#include "BSDF.h"
#include "Math/Random.h"

#pragma optimize("", off)

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

void OrenNayarBSDF::Sample(const math::Vector4& outgoingDir, math::Vector4& outIncomingDir, math::Vector4& outWeight, math::Random& randomGenerator) const
{
    RT_UNUSED(outgoingDir);

    outIncomingDir = randomGenerator.GetHemishpereCos();

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

    outWeight = VECTOR_ONE;
}

math::Vector4 OrenNayarBSDF::Evaluate(const math::Vector4& outgoingDir, const math::Vector4& incomingDir) const
{
    RT_UNUSED(outgoingDir);

    const float NdotL = incomingDir[2];
    return Vector4(NdotL);
}

/////////////////////////////////////////////////////////

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

void CookTorranceBSDF::Sample(const math::Vector4& outgoingDir, math::Vector4& outIncomingDir, math::Vector4& outWeight, math::Random& randomGenerator) const
{
    const float u0 = randomGenerator.GetFloat();
    const float u1 = randomGenerator.GetFloat();

    // generate microfacet normal vector using GGX distribution function (Trowbridge-Reitz)
    const float a = mRougness * mRougness;
    const float cosThetaSqr = (1.0f - u0) / (1.0f + (a * a - 1.0f) * u0);
    const float cosTheta = sqrtf(cosThetaSqr);
    const float sinTheta = sqrtf(1.0f - cosThetaSqr);
    const float phi = 2.0f * RT_PI * u1;

    // microfacet normal (aka. half vector)
    const Vector4 m(sinTheta * cosf(phi), sinTheta * sinf(phi), cosTheta, 0.0f);

    outIncomingDir = -Vector4::Reflect3(outgoingDir, m);

    const float NdotH = m[2];
    const float NdotV = fabsf(outgoingDir[2]);
    const float NdotL = fabsf(outIncomingDir[2]);

    // clip the function to avoid division by zero
    if (NdotV <= FLT_EPSILON || NdotL <= FLT_EPSILON)
    {
        outWeight = Vector4();
        return;
    }

    // Geometry term
    const float G1 = NdotH * NdotV;
    const float G2 = NdotH * NdotL;
    const float G = Min(1.0f, Max(0.0f, 2.0f * Min(G1, G2)));

    outWeight = Vector4(G / NdotV);
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

/////////////////////////////////////////////////////////

Vector4 Refract(const Vector4& incidentVec, float eta)
{
    float NdotV = incidentVec.z;
    if (NdotV < 0.0f)
    {
        eta = 1.0f / eta;
    }

    const float k = 1.0f - eta * eta * (1.0f - NdotV * NdotV);

    assert(k >= 0.0f);
    if (k < 0.0f)
    {
        return Vector4();
    }

    Vector4 transmitted = incidentVec * eta - (eta * NdotV + sqrtf(k)) * VECTOR_Z;
    assert(fabsf(1.0f - transmitted.Length3()) > 0.01f);

    if (NdotV > 0.0f)
    {
        transmitted.z = -transmitted.z;
    }

    return transmitted.Normalized3();
}

TransparencyBSDF::TransparencyBSDF(float ior)
    : IOR(ior)
{
}

void TransparencyBSDF::Sample(const math::Vector4& outgoingDir, math::Vector4& outIncomingDir, math::Vector4& outWeight, math::Random& randomGenerator) const
{
    RT_UNUSED(randomGenerator);

    outIncomingDir = Refract(-outgoingDir, IOR);
    outWeight = VECTOR_ONE;
}

Vector4 TransparencyBSDF::Evaluate(const math::Vector4& outgoingDir, const math::Vector4& incomingDir) const
{
    RT_UNUSED(outgoingDir);
    RT_UNUSED(incomingDir);

    return Vector4();
}

} // namespace rt
