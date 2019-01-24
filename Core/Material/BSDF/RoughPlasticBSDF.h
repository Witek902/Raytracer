#pragma once

#include "BSDF.h"

namespace rt {

// Rough plastic-like BSDF
class RoughPlasticBSDF : public BSDF
{
public:
    virtual const char* GetName() const override;
    virtual bool Sample(SamplingContext& ctx) const override;
    virtual const Color Evaluate(const EvaluationContext& ctx, Float* outDirectPdfW = nullptr) const override;
};

} // namespace rt
