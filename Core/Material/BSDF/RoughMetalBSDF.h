#pragma once

#include "BSDF.h"

namespace rt {

// Rough metal (conductor) BRDF.
class RoughMetalBSDF : public BSDF
{
public:
    virtual const char* GetName() const override;
    virtual bool IsDelta() const override { return false; } // TODO depends on material
    virtual bool Sample(SamplingContext& ctx) const override;
    virtual const RayColor Evaluate(const EvaluationContext& ctx, Float* outDirectPdfW = nullptr, Float* outReversePdfW = nullptr) const override;
    virtual Float Pdf(const EvaluationContext& ctx, PdfDirection dir) const override;
};

} // namespace rt
