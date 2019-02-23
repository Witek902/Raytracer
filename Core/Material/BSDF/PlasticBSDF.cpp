#include "PCH.h"
#include "PlasticBSDF.h"
#include "Math/Random.h"
#include "Math/Utils.h"

namespace rt {

using namespace math;

const char* PlasticBSDF::GetName() const
{
    return "plastic";
}

bool PlasticBSDF::Sample(SamplingContext& ctx) const
{
    const float NdotV = ctx.outgoingDir.z;
    if (NdotV < CosEpsilon)
    {
        return false;
    }

    const float ior = ctx.materialParam.IoR;
    const float eta = 1.0f / ior;

    const float Fi = FresnelDielectric(NdotV, ior);

    const float specularWeight = Fi;
    const float diffuseWeight = (1.0f - Fi) * ctx.materialParam.baseColor.Max();

    // importance sample specular reflectivity
    const float specularProbability = specularWeight / (specularWeight + diffuseWeight);
    const float diffuseProbability = 1.0f - specularProbability;
    const bool specular = ctx.randomGenerator.GetFloat() < specularProbability;

    if (specular)
    {
        ctx.outColor = RayColor(Fi / specularProbability);
        ctx.outIncomingDir = -Vector4::Reflect3(ctx.outgoingDir, VECTOR_Z);
        ctx.outPdf = specularProbability;
        ctx.outEventType = SpecularReflectionEvent;
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

const RayColor PlasticBSDF::Evaluate(const EvaluationContext& ctx, float* outDirectPdfW, float* outReversePdfW) const
{
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

    const float specularProbability = specularWeight / (specularWeight + diffuseWeight);
    const float diffuseProbability = 1.0f - specularProbability;

    if (outDirectPdfW)
    {
        // cos-weighted hemisphere distribution
        *outDirectPdfW = NdotL * RT_INV_PI * diffuseProbability;
    }

    if (outReversePdfW)
    {
        // cos-weighted hemisphere distribution
        *outReversePdfW = NdotV * RT_INV_PI * diffuseProbability;
    }

    return ctx.materialParam.baseColor * (NdotL * RT_INV_PI * (1.0f - Fi) * (1.0f - Fo));
}

float PlasticBSDF::Pdf(const EvaluationContext& ctx, PdfDirection dir) const
{
    const float NdotV = ctx.outgoingDir.z;
    const float NdotL = -ctx.incomingDir.z;

    if (NdotV < CosEpsilon || NdotL < CosEpsilon)
    {
        return 0.0f;
    }

    const float Fi = FresnelDielectric(NdotV, ctx.materialParam.IoR);

    const float specularWeight = Fi;
    const float diffuseWeight = (1.0f - Fi) * ctx.materialParam.baseColor.Max();

    const float specularProbability = specularWeight / (specularWeight + diffuseWeight);
    const float diffuseProbability = 1.0f - specularProbability;

    if (dir == ForwardPdf)
    {
        return NdotL * RT_INV_PI * diffuseProbability;
    }
    else
    {
        return NdotV * RT_INV_PI * diffuseProbability;
    }
}

} // namespace rt
