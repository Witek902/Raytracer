#include "PCH.h"
#include "RoughDiffuseBSDF.h"
#include "Math/Random.h"

namespace rt {

using namespace math;

const char* RoughDiffuseBSDF::GetName() const
{
    return "roughDiffuse";
}

float RoughDiffuseBSDF::Evaluate_Internal(const float NdotL, const float NdotV, const float LdotV, const float roughness)
{
    // based on http://mimosa-pudica.net/improved-oren-nayar.html
    const float s2 = roughness * roughness;
    const float A = 1.0f - 0.50f * s2 / (0.33f + s2);
    const float B =        0.45f * s2 / (0.09f + s2);
    const float s = LdotV - NdotL * NdotV;
    const float stinv = s > 0.0f ? s / Max(NdotL, NdotV) : 0.0f;

    return Max(A + B * stinv, 0.0f);
}

bool RoughDiffuseBSDF::Sample(SamplingContext& ctx) const
{
    const float NdotV = ctx.outgoingDir.z;
    if (NdotV < CosEpsilon)
    {
        return false;
    }

    ctx.outIncomingDir = ctx.randomGenerator.GetHemishpereCos();

    const float NdotL = ctx.outIncomingDir.z;
    const float LdotV = Max(0.0f, Vector4::Dot3(ctx.outgoingDir, -ctx.outIncomingDir));
    const float value = Evaluate_Internal(NdotL, NdotV, LdotV, ctx.materialParam.roughness);

    ctx.outPdf = NdotL * RT_INV_PI;
    ctx.outColor = ctx.materialParam.baseColor * value;
    ctx.outEventType = DiffuseReflectionEvent;

    return true;
}

const RayColor RoughDiffuseBSDF::Evaluate(const EvaluationContext& ctx, float* outDirectPdfW, Float* outReversePdfW) const
{
    const float NdotV = ctx.outgoingDir.z;
    const float NdotL = -ctx.incomingDir.z;

    if (NdotV > CosEpsilon && NdotL > CosEpsilon)
    {
        if (outDirectPdfW)
        {
            // cos-weighted hemisphere distribution
            *outDirectPdfW = NdotL * RT_INV_PI;
        }

        if (outReversePdfW)
        {
            // cos-weighted hemisphere distribution
            *outDirectPdfW = NdotV * RT_INV_PI;
        }

        const float LdotV = Max(0.0f, Vector4::Dot3(ctx.outgoingDir, -ctx.incomingDir));
        const float value = NdotL * RT_INV_PI * Evaluate_Internal(NdotL, NdotV, LdotV, ctx.materialParam.roughness);

        return ctx.materialParam.baseColor * value;
    }

    return RayColor::Zero();
}

Float RoughDiffuseBSDF::Pdf(const EvaluationContext& ctx, PdfDirection dir) const
{
    const float NdotV = ctx.outgoingDir.z;
    const Float NdotL = -ctx.incomingDir.z;

    if (NdotV > CosEpsilon && NdotL > CosEpsilon)
    {
        if (dir == ForwardPdf)
        {
            return NdotL * RT_INV_PI;
        }
        else
        {
            return NdotV * RT_INV_PI;
        }
    }

    return 0.0f;
}

} // namespace rt
