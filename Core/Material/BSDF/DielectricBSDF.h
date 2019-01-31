#pragma once

#include "BSDF.h"

namespace rt {

// Smooth transparent dielectic BSDF (e.g. polished glass or surface of water).
class DielectricBSDF : public BSDF
{
public:
    virtual const char* GetName() const override;
    virtual bool Sample(SamplingContext& ctx) const override;
    virtual const RayColor Evaluate(const EvaluationContext& ctx, Float* outDirectPdfW = nullptr) const override;
};

} // namespace rt
