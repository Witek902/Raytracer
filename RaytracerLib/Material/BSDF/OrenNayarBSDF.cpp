#include "PCH.h"
#include "OrenNayarBSDF.h"
#include "Math/Random.h"

namespace rt {

using namespace math;

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

} // namespace rt
