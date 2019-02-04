#include "PCH.h"
#include "Microfacet.h"
#include "RoughPlasticBSDF.h"
#include "PlasticBSDF.h"
#include "../Material.h"
#include "Math/Utils.h"

namespace rt {

using namespace math;

const char* RoughPlasticBSDF::GetName() const
{
    return "roughPlastic";
}

bool RoughPlasticBSDF::Sample(SamplingContext& ctx) const
{
    const float NdotV = ctx.outgoingDir.z;
    if (NdotV < CosEpsilon)
    {
        return false;
    }

    const Float roughness = ctx.materialParam.roughness;

    // fallback to specular event
    if (roughness < SpecularEventRoughnessTreshold)
    {
        return PlasticBSDF().Sample(ctx);
    }

    const float ior = ctx.materialParam.IoR;
    const float eta = 1.0f / ior;

    const float Fi = FresnelDielectric(NdotV, ior);

    const float specularWeight = Fi;
    const float diffuseWeight = (1.0f - Fi) * ctx.materialParam.baseColor.Max();
    RT_ASSERT(diffuseWeight >= 0.0f && diffuseWeight <= 1.0f);

    // importance sample specular reflectivity
    const float specularProbability = specularWeight / (specularWeight + diffuseWeight);
    const float diffuseProbability = 1.0f - specularProbability;
    const bool specular = ctx.randomGenerator.GetFloat() < specularProbability;

    if (specular)
    {
        // microfacet normal (aka. half vector)
        const Microfacet microfacet(roughness * roughness);
        const Vector4 m = microfacet.Sample(ctx.randomGenerator);

        // compute reflected direction
        ctx.outIncomingDir = -Vector4::Reflect3(ctx.outgoingDir, m);

        const float NdotL = ctx.outIncomingDir.z;
        const float VdotH = Vector4::Dot3(m, ctx.outgoingDir);

        if (NdotL < CosEpsilon || VdotH < CosEpsilon)
        {
            return false;
        }

        const float pdf = microfacet.Pdf(m);
        const float D = microfacet.D(m);
        const float G = microfacet.G(NdotV, NdotL);
        const float F = FresnelDielectric(VdotH, ctx.material.IoR);

        ctx.outPdf = pdf / (4.0f * VdotH) * specularProbability;
        ctx.outColor = RayColor(VdotH * F * G * D / (pdf * NdotV * specularProbability));
        ctx.outEventType = GlossyReflectionEvent;

        RT_ASSERT(ctx.outPdf > 0.0f);
        RT_ASSERT(ctx.outColor.IsValid());
    }
    else // diffuse reflection
    {
        ctx.outIncomingDir = ctx.randomGenerator.GetHemishpereCos();
        const float NdotL = ctx.outIncomingDir.z;

        ctx.outPdf = ctx.outIncomingDir.z * RT_INV_PI * diffuseProbability;

        const float Fo = FresnelDielectric(NdotL, ior);
        ctx.outColor = ctx.materialParam.baseColor * ((1.0f - Fi) * (1.0f - Fo) / diffuseProbability);

        ctx.outEventType = DiffuseReflectionEvent;
    }

    return true;
}

const RayColor RoughPlasticBSDF::Evaluate(const EvaluationContext& ctx, Float* outDirectPdfW, Float* outReversePdfW) const
{
    const Float roughness = ctx.materialParam.roughness;

    // fallback to specular event
    if (roughness < SpecularEventRoughnessTreshold)
    {
        return PlasticBSDF().Evaluate(ctx, outDirectPdfW);
    }

    const float NdotV = ctx.outgoingDir.z;
    const float NdotL = -ctx.incomingDir.z;

    if (NdotV < CosEpsilon || NdotL < CosEpsilon)
    {
        return RayColor::Zero();
    }

    const float ior = ctx.materialParam.IoR;

    const float Fi = FresnelDielectric(NdotV, ior);
    const float Fo = FresnelDielectric(NdotL, ior);

    const float specularWeight = Fi;
    const float diffuseWeight = (1.0f - Fi) * ctx.materialParam.baseColor.Max();
    RT_ASSERT(diffuseWeight >= 0.0f && diffuseWeight <= 1.0f);

    const float specularProbability = specularWeight / (specularWeight + diffuseWeight);
    const float diffuseProbability = 1.0f - specularProbability;

    float diffusePdf = NdotL * RT_INV_PI; // cos-weighted hemisphere distribution
    float diffuseReversePdf = NdotV * RT_INV_PI;
    float specularPdf = 0.0f;

    RayColor diffuseTerm = ctx.materialParam.baseColor * (NdotL * RT_INV_PI * (1.0f - Fi) * (1.0f - Fo));
    RayColor specularTerm = RayColor::Zero();

    {
        // microfacet normal
        const Vector4 m = (ctx.outgoingDir - ctx.incomingDir).Normalized3();
        const float VdotH = Vector4::Dot3(m, ctx.outgoingDir);

        // clip the function
        if (VdotH >= CosEpsilon)
        {
            const Microfacet microfacet(roughness * roughness);
            const float D = microfacet.D(m);
            const float G = microfacet.G(NdotV, NdotL);
            const float F = FresnelDielectric(VdotH, ctx.material.IoR);

            specularPdf = microfacet.Pdf(m) / (4.0f * VdotH);
            specularTerm = RayColor(F * G * D / (4.0f * NdotV));
        }
    }

    if (outDirectPdfW)
    {
        *outDirectPdfW = diffusePdf * diffuseProbability + specularPdf * specularProbability;
    }

    if (outReversePdfW)
    {
        *outDirectPdfW = diffuseReversePdf * diffuseProbability + specularPdf * specularProbability;
    }

    return diffuseTerm + specularTerm;
}

Float RoughPlasticBSDF::Pdf(const EvaluationContext& ctx, PdfDirection dir) const
{
    const Float roughness = ctx.materialParam.roughness;

    // fallback to specular event
    if (roughness < SpecularEventRoughnessTreshold)
    {
        return PlasticBSDF().Pdf(ctx, dir);
    }

    const float NdotV = ctx.outgoingDir.z;
    const float NdotL = -ctx.incomingDir.z;

    if (NdotV < CosEpsilon || NdotL < CosEpsilon)
    {
        return 0.0f;
    }

    const float Fi = FresnelDielectric(NdotV, ctx.materialParam.IoR);

    const float specularWeight = Fi;
    const float diffuseWeight = (1.0f - Fi) * ctx.materialParam.baseColor.Max();
    RT_ASSERT(diffuseWeight >= 0.0f && diffuseWeight <= 1.0f);

    const float specularProbability = specularWeight / (specularWeight + diffuseWeight);
    const float diffuseProbability = 1.0f - specularProbability;

    // cos-weighted hemisphere distribution
    float diffusePdf;
    if (dir == ForwardPdf)
    {
        diffusePdf = NdotL * RT_INV_PI;
    }
    else
    {
        diffusePdf = NdotV * RT_INV_PI;
    }

    float specularPdf = 0.0f;

    // microfacet normal
    const Vector4 m = (ctx.outgoingDir - ctx.incomingDir).Normalized3();
    const float VdotH = Vector4::Dot3(m, ctx.outgoingDir);

    // clip the function
    if (VdotH >= CosEpsilon)
    {
        const Microfacet microfacet(roughness * roughness);
        const float F = FresnelDielectric(VdotH, ctx.material.IoR);

        specularPdf = microfacet.Pdf(m) / (4.0f * VdotH);
    }

    return diffusePdf * diffuseProbability + specularPdf * specularProbability;
}

} // namespace rt
