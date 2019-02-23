#pragma once

#include "BSDF.h"

namespace rt {

// Smooth metal (conductor) BRDF.
class MetalBSDF : public BSDF
{
public:
    virtual const char* GetName() const override;
    virtual bool IsDelta() const override { return true; }
    virtual bool Sample(SamplingContext& ctx) const override;
    virtual const RayColor Evaluate(const EvaluationContext& ctx, float* outDirectPdfW = nullptr, float* outReversePdfW = nullptr) const override;
    virtual float Pdf(const EvaluationContext& ctx, PdfDirection dir) const override;
};

} // namespace rt
