#include "PCH.h"
#include "DiffuseBSDF.h"
#include "Math/Random.h"

namespace rt {

using namespace math;

const char* DiffuseBSDF::GetName() const
{
    return "diffuse";
}

bool DiffuseBSDF::Sample(SamplingContext& ctx) const
{
    const float NdotV = ctx.outgoingDir.z;

    if (NdotV < CosEpsilon)
    {
        return false;
    }

    ctx.outIncomingDir = ctx.randomGenerator.GetHemishpereCos();
    ctx.outPdf = ctx.outIncomingDir.z * RT_INV_PI;
    ctx.outColor = ctx.materialParam.baseColor;
    ctx.outEventType = DiffuseReflectionEvent;

    return true;
}

const RayColor DiffuseBSDF::Evaluate(const EvaluationContext& ctx, float* outDirectPdfW, float* outReversePdfW) const
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
            *outReversePdfW = NdotV * RT_INV_PI;
        }

        return ctx.materialParam.baseColor * RayColor(NdotL * RT_INV_PI);
    }

    return RayColor::Zero();
}

float DiffuseBSDF::Pdf(const EvaluationContext& ctx, PdfDirection dir) const
{
    const float NdotV = ctx.outgoingDir.z;
    const float NdotL = -ctx.incomingDir.z;

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
