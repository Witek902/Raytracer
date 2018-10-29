#include "PCH.h"
#include "LambertianBSDF.h"
#include "Math/Random.h"

namespace rt {

using namespace math;

bool LambertianBSDF::Sample(SamplingContext& ctx) const
{
    if (ctx.outgoingDir.z < CosEpsilon)
    {
        return false;
    }

    ctx.outIncomingDir = ctx.randomGenerator.GetHemishpereCos();
    ctx.outPdf = ctx.outIncomingDir.z * RT_INV_PI;
    ctx.outColor = Color(ctx.outIncomingDir.z * RT_INV_PI);
    ctx.outEventType = DiffuseReflectionEvent;

    return true;
}

const Color LambertianBSDF::Evaluate(const EvaluationContext& ctx, Float* outDirectPdfW) const
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

        return Color(NdotL * RT_INV_PI);
    }

    return Color();
}

} // namespace rt
