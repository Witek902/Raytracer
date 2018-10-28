#include "PCH.h"
#include "SpecularTransmissiveBSDF.h"
#include "../Material.h"

namespace rt {

using namespace math;

bool SpecularTransmissiveBSDF::Sample(SamplingContext& ctx) const
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

const Vector4 SpecularTransmissiveBSDF::Evaluate(const EvaluationContext& ctx, Float* outDirectPdfW) const
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
