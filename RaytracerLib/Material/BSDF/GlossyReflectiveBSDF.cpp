#include "PCH.h"
#include "Microfacet.h"
#include "GlossyReflectiveBSDF.h"
#include "Math/Random.h"

namespace rt {

using namespace math;

bool GlossyReflectiveBSDF::Sample(SamplingContext& ctx) const
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

    const Microfacet microfacet(roughness * roughness);

    // microfacet normal (aka. half vector)
    const Vector4 m = microfacet.Sample(ctx.randomGenerator);

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

    const float pdf = microfacet.Pdf(m);
    const float D = microfacet.D(m);
    const float G = microfacet.G(NdotV, NdotL);

    ctx.outPdf = pdf / (4.0f * VdotH);
    ctx.outColor = Color(G * D / (4.0f * NdotV));
    ctx.outEventType = GlossyReflectionEvent;

    return true;
}

const Color GlossyReflectiveBSDF::Evaluate(const EvaluationContext& ctx, Float* outDirectPdfW) const
{
    const Float roughness = ctx.materialParam.roughness;

    // fallback to specular event
    if (roughness < 0.01f)
    {
        if (outDirectPdfW)
        {
            *outDirectPdfW = 0.0f;
        }
        return Color();
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
        return Color();
    }

    const Microfacet microfacet(roughness * roughness);
    const float D = microfacet.D(m);
    const float G = microfacet.G(NdotV, NdotL);

    if (outDirectPdfW)
    {
        *outDirectPdfW = microfacet.Pdf(m) / (4.0f * VdotH);
    }

    return Color(G * D / (4.0f * NdotV));
}

} // namespace rt
