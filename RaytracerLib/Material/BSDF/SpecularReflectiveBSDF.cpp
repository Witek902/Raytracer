#include "PCH.h"
#include "SpecularReflectiveBSDF.h"


namespace rt {

using namespace math;

bool SpecularReflectiveBSDF::Sample(SamplingContext& ctx) const
{
    if (ctx.outgoingDir.z < CosEpsilon)
    {
        return false;
    }

    ctx.outColor = Color::One();
    ctx.outIncomingDir = -Vector4::Reflect3(ctx.outgoingDir, VECTOR_Z);
    ctx.outPdf = 1.0f;
    ctx.outEventType = SpecularReflectionEvent;
    return true;
}

const Color SpecularReflectiveBSDF::Evaluate(const EvaluationContext& ctx, Float* outDirectPdfW) const
{
    RT_UNUSED(ctx);

    if (outDirectPdfW)
    {
        *outDirectPdfW = 0.0f;
    }
    return Color();
}

} // namespace rt
