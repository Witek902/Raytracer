#pragma once

#include "BSDF.h"

namespace rt {

class RoughDiffuseBSDF : public BSDF
{
public:
    virtual const char* GetName() const override;
    virtual bool Sample(SamplingContext& ctx) const override;
    virtual const Color Evaluate(const EvaluationContext& ctx, Float* outDirectPdfW = nullptr) const override;

    static float Evaluate_Internal(const float NdotL, const float NdotV, const float LdotV, const float roughness);
};

} // namespace rt
