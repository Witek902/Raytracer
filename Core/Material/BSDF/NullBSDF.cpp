#include "PCH.h"
#include "NullBSDF.h"

namespace rt {

const char* NullBSDF::GetName() const
{
    return "null";
}

bool NullBSDF::Sample(SamplingContext& ctx) const
{
    RT_UNUSED(ctx);

    return false;
}

const RayColor NullBSDF::Evaluate(const EvaluationContext& ctx, Float* outDirectPdfW, Float* outReversePdfW) const
{
    RT_UNUSED(ctx);
    RT_UNUSED(outDirectPdfW);
    RT_UNUSED(outReversePdfW);

    return RayColor::Zero();
}

Float NullBSDF::Pdf(const EvaluationContext& ctx, PdfDirection dir) const
{
    RT_UNUSED(ctx);
    RT_UNUSED(dir);

    return 0.0f;
}

} // namespace rt
