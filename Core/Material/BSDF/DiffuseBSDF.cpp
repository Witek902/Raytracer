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

const Color DiffuseBSDF::Evaluate(const EvaluationContext& ctx, Float* outDirectPdfW) const
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

        return ctx.materialParam.baseColor * Color(NdotL * RT_INV_PI);
    }

    return Color::Zero();
}

} // namespace rt
