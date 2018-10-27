#include "PCH.h"
#include "BSDF.h"
#include "Material.h"
#include "Math/Random.h"
#include "Math/Transcendental.h"
#include "Color/Color.h"


namespace rt {

using namespace math;

static constexpr Float CosEpsilon = 1.0e-5f;

// GGX microfacet model
class Microfacet
{
public:
    static float D(const Vector4& m, const float a)
    {
        const float NdotH = m.z;
        const float a2 = a * a;

        float cosThetaSq = Sqr(NdotH);
        float tanThetaSq = Max(1.0f - cosThetaSq, 0.0f) / cosThetaSq;
        float cosThetaQu = cosThetaSq * cosThetaSq;
        return a2 * RT_INV_PI / (cosThetaQu * Sqr(a2 + tanThetaSq));
    }

    static float Pdf(const Vector4& m, const float a)
    {
        return D(m, a) * m.z;
    }

    static float G1(const float NdotX, const float a)
    {
        const float a2 = a * a;

        float tanThetaSq = Max(1.0f - NdotX * NdotX, 0.0f) / (NdotX * NdotX);
        return 2.0f / (1.0f + Sqrt(1.0f + a2 * tanThetaSq));
    }

    static float G(const float NdotV, const float NdotL, const float a)
    {
        return G1(NdotV, a) * G1(NdotL, a);
    }

    static const Vector4 Sample(Random& randomGenerator, const float a)
    {
        const float a2 = a * a;

        // generate microfacet normal vector using GGX distribution function (Trowbridge-Reitz)
        const Float2 u = randomGenerator.GetFloat2();
        const float cosThetaSqr = (1.0f - u.x) / (1.0f + (a2 - 1.0f) * u.x);
        const float cosTheta = Sqrt(cosThetaSqr);
        const float sinTheta = Sqrt(1.0f - cosThetaSqr);
        const float phi = 2.0f * RT_PI * u.y;

        return Vector4(sinTheta * cosf(phi), sinTheta * sinf(phi), cosTheta, 0.0f);
    }
};

bool OrenNayarBSDF::Sample(SamplingContext& ctx) const
{
    if (ctx.outgoingDir.z < CosEpsilon)
    {
        return false;
    }

    ctx.outIncomingDir = ctx.randomGenerator.GetHemishpereCos();
    ctx.outPdf = ctx.outIncomingDir.z * RT_INV_PI;
    ctx.outColor = Color::One() * ctx.outIncomingDir.z * RT_INV_PI;
    ctx.outEventType = DiffuseReflectionEvent;

    /*
    const float NdotV = ctx.outgoingDir.z;
    const float NdotL = outIncomingDir.z;

    if (NdotV > CosEpsilon || NdotL > CosEpsilon)
    {
        const float LdotV = Max(0.0f, Vector4::Dot3(ctx.outgoingDir, -outIncomingDir));

        const float s2 = mRoughness * mRoughness;
        const float A = 1.0f - 0.50f * s2 / (0.33f + s2);
        const float B = 0.45f * s2 / (0.09f + s2);

        // based on http://mimosa-pudica.net/improved-oren-nayar.html
        const float s = LdotV - NdotL * NdotV;
        const float stinv = s > 0.0f ? s / Max(NdotL, NdotV) : 0.0f;

        outPdf = outIncomingDir.z / RT_PI;
        return Color::One() * Max(A + B * stinv, 0.0f);
    }

    return Color();
    */

    return true;
}

const Vector4 OrenNayarBSDF::Evaluate(const EvaluationContext& ctx, Float* outDirectPdfW) const
{
    const float NdotV = ctx.outgoingDir.z;
    const float NdotL = -ctx.incomingDir.z;

    if (NdotV > CosEpsilon || NdotL > CosEpsilon)
    {
        if (outDirectPdfW)
        {
            // cos-weighted hemisphere distribution
            *outDirectPdfW = NdotL * RT_INV_PI;
        }

        return Vector4(NdotL * RT_INV_PI);

        /*
        const float LdotV = Max(0.0f, Vector4::Dot3(ctx.outgoingDir, -ctx.incomingDir));

        const float s2 = mRoughness * mRoughness;
        const float A = 1.0f - 0.50f * s2 / (0.33f + s2);
        const float B =        0.45f * s2 / (0.09f + s2);

        // based on http://mimosa-pudica.net/improved-oren-nayar.html
        const float s = LdotV - NdotL * NdotV;
        const float stinv = s > 0.0f ? s / Max(NdotL, NdotV) : 0.0f;

        return Vector4(Max(RT_PI * NdotL * (A + B * stinv), 0.0f));
        */
    }

    return Vector4();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CookTorranceBSDF::Sample(SamplingContext& ctx) const
{
    if (ctx.outgoingDir.z < CosEpsilon)
    {
        return false;
    }

    const Float roughness = ctx.materialParam.roughness;

    // fallback to specular event
    if (roughness < 0.01f)
    {
        ctx.outColor = Color::One();
        ctx.outIncomingDir = -Vector4::Reflect3(ctx.outgoingDir, VECTOR_Z);
        ctx.outPdf = 1.0f;
        ctx.outEventType = SpecularReflectionEvent;
        return true;
    }

    const float a = roughness * roughness;

    // microfacet normal (aka. half vector)
    const Vector4 m = Microfacet::Sample(ctx.randomGenerator, a);

    // compute reflected direction
    ctx.outIncomingDir = -Vector4::Reflect3(ctx.outgoingDir, m);
    if (ctx.outIncomingDir.z < CosEpsilon)
    {
        return false;
    }

    const float NdotH = m.z;
    const float NdotV = ctx.outgoingDir.z;
    const float NdotL = ctx.outIncomingDir.z;
    const float VdotH = Vector4::Dot3(m, ctx.outgoingDir);

    // clip the function to avoid division by zero
    if (NdotV < CosEpsilon || NdotL < CosEpsilon)
    {
        return false;
    }

    const float pdf = Microfacet::Pdf(m, a);
    const float D = Microfacet::D(m, a);
    const float G = Microfacet::G(NdotV, NdotL, a);

    ctx.outPdf = pdf / (4.0f * VdotH);
    ctx.outColor = Color::One() * (G * D / (4.0f * NdotV));
    ctx.outEventType = GlossyReflectionEvent;

    return true;
}

const Vector4 CookTorranceBSDF::Evaluate(const EvaluationContext& ctx, Float* outDirectPdfW) const
{
    const Float roughness = ctx.materialParam.roughness;

    // fallback to specular event
    if (roughness < 0.01f)
    {
        if (outDirectPdfW)
        {
            *outDirectPdfW = 0.0f;
        }
        return Vector4();
    }

    // microfacet normal
    const Vector4 m = (ctx.outgoingDir - ctx.incomingDir).FastNormalized3();

    const float NdotH = m.z;
    const float NdotV = ctx.outgoingDir.z;
    const float NdotL = -ctx.incomingDir.z;
    const float VdotH = Vector4::Dot3(m, ctx.outgoingDir);

    // clip the function
    if (NdotV < CosEpsilon || NdotL < CosEpsilon)
    {
        return Vector4();
    }

    const float a = roughness * roughness;
    const float D = Microfacet::D(m, a);
    const float G = Microfacet::G(NdotV, NdotL, a);

    if (outDirectPdfW)
    {
        *outDirectPdfW = Microfacet::Pdf(m, a) / (4.0f * VdotH);
    }

    return Vector4(G * D / (4.0f * NdotV));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool TransparencyBSDF::Sample(SamplingContext& ctx) const
{
    Color color = Color::One();
    float ior = ctx.materialParam.IoR;

    if (ctx.material.isDispersive)
    {
        const float* B = ctx.material.dispersionParams.B;
        const float* C = ctx.material.dispersionParams.C;
        const float lambda = 1.0e+6f * Wavelength::Lower + ctx.wavelength.GetBase() * (Wavelength::Higher - Wavelength::Lower);
        const float lambda2 = lambda * lambda;
        ior = Sqrt(1.0f + B[0] * lambda2 / (lambda2 - C[0]) + B[1] * lambda2 / (lambda2 - C[1]) + B[2] * lambda2 / (lambda2 - C[2]));

        if (!ctx.wavelength.isSingle)
        {
            color = Color::SingleWavelengthFallback();
            ctx.wavelength.isSingle = true;
        }
    }

    ctx.outPdf = 1.0f;
    ctx.outIncomingDir = Vector4::RefractZ(-ctx.outgoingDir, ior);
    ctx.outColor = color;
    ctx.outEventType = SpecularRefractionEvent;

    return true;
}

const Vector4 TransparencyBSDF::Evaluate(const EvaluationContext& ctx, Float* outDirectPdfW) const
{
    RT_UNUSED(ctx);
    RT_UNUSED(outDirectPdfW);

    if (outDirectPdfW)
    {
        // TODO
        *outDirectPdfW = 0.0f;
    }

    return Vector4();
}

} // namespace rt
