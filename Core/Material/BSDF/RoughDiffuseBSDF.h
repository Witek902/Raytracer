#pragma once

#include "BSDF.h"

namespace rt {

class RoughDiffuseBSDF : public BSDF
{
public:
    virtual const char* GetName() const override;
    virtual bool IsDelta() const override { return false; }
    virtual bool Sample(SamplingContext& ctx) const override;
    virtual const RayColor Evaluate(const EvaluationContext& ctx, Float* outDirectPdfW = nullptr, Float* outReversePdfW = nullptr) const override;
    virtual Float Pdf(const EvaluationContext& ctx, PdfDirection dir) const override;

    static float Evaluate_Internal(const float NdotL, const float NdotV, const float LdotV, const float roughness);
};

} // namespace rt
