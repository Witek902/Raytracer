#include "PCH.h"
#include "DielectricBSDF.h"
#include "../Material.h"
#include "Math/Utils.h"

namespace rt {

using namespace math;

const char* DielectricBSDF::GetName() const
{
    return "dielectric";
}

bool DielectricBSDF::Sample(SamplingContext& ctx) const
{
    const float NdotV = ctx.outgoingDir.z;
    if (Abs(NdotV) < CosEpsilon)
    {
        return false;
    }

    float ior = ctx.materialParam.IoR;

    bool fallbackToSingleWavelength = false;

    // handle dispersion
#ifdef RT_ENABLE_SPECTRAL_RENDERING
    if (ctx.material.isDispersive)
    {
        const float lambda = 1.0e+6f * (Wavelength::Lower + ctx.wavelength.GetBase() * (Wavelength::Higher - Wavelength::Lower));
        // Cauchy's equation for light dispersion
        const float lambda2 = lambda * lambda;
        const float lambda4 = lambda2 * lambda2;
        ior += ctx.material.dispersionParams.C / lambda2;
        ior += ctx.material.dispersionParams.D / lambda4;
        if (!ctx.wavelength.isSingle)
        {
            fallbackToSingleWavelength = true;
            ctx.wavelength.isSingle = true;
        }
    }
#endif // RT_ENABLE_SPECTRAL_RENDERING

    // compute Fresnel term
    const float F = FresnelDielectric(NdotV, ior);

    // increase probability of sampling refraction
    // this helps reducing noise in reflections in dielectrics when refracted ray returns dark color
    const float minReflectionProbability = 0.25f;
    const float reflectionProbability = minReflectionProbability + (1.0f - minReflectionProbability) * F;
    const float refractionProbability = 1.0f - reflectionProbability;

    // sample event
    const bool reflection = (reflectionProbability >= 1.0f) || ctx.sample.x < reflectionProbability;
    if (reflection) 
    {
        ctx.outIncomingDir = -Vector4::Reflect3(ctx.outgoingDir, VECTOR_Z);
        ctx.outEventType = SpecularReflectionEvent;
    }
    else // transmission
    {
        ctx.outIncomingDir = Vector4::Refract3(-ctx.outgoingDir, VECTOR_Z, ior);
        ctx.outEventType = SpecularRefractionEvent;
    }

    const float NdotL = ctx.outIncomingDir.z;

    if ((NdotV * NdotL > 0.0f) != reflection)
    {
        // discard samples that land on wrong surface side
        return false;
    }

    if (reflection)
    {
        ctx.outPdf = reflectionProbability;
        ctx.outColor = RayColor::One();

        if (minReflectionProbability > 0.0f)
        {
            ctx.outColor *= F / reflectionProbability;
        }
    }
    else
    {
        ctx.outPdf = refractionProbability;
        ctx.outColor = ctx.materialParam.baseColor;

        if (minReflectionProbability > 0.0f)
        {
            ctx.outColor *= (1.0f - F) / refractionProbability;
        }
    }

    if (fallbackToSingleWavelength)
    {
        // in case of wavelength-dependent event, switch to single wavelength per ray
        ctx.outColor *= RayColor::SingleWavelengthFallback();
    }

    return true;
}

const RayColor DielectricBSDF::Evaluate(const EvaluationContext& ctx, float* outDirectPdfW, float* outReversePdfW) const
{
    RT_UNUSED(ctx);

    // Dirac delta, assume we cannot hit it

    if (outDirectPdfW)
    {
        *outDirectPdfW = 0.0f;
    }

    if (outReversePdfW)
    {
        *outReversePdfW = 0.0f;
    }

    return RayColor::Zero();
}

float DielectricBSDF::Pdf(const EvaluationContext& ctx, PdfDirection dir) const
{
    RT_UNUSED(ctx);
    RT_UNUSED(dir);

    // Dirac delta, assume we cannot hit it
    return 0.0f;
}

} // namespace rt
