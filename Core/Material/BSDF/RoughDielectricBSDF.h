#pragma once

#include "BSDF.h"

namespace rt {

// Rough transparent dielectic BSDF (e.g. ground glass).
class RoughDielectricBSDF : public BSDF
{
public:
    virtual const char* GetName() const override;
    virtual bool Sample(SamplingContext& ctx) const override;
    virtual const RayColor Evaluate(const EvaluationContext& ctx, Float* outDirectPdfW = nullptr) const override;
};

} // namespace rt
