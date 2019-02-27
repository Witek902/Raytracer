#pragma once

#include "../../Math/Transcendental.h"

namespace rt {

// TODO anisotropy

// GGX microfacet model
class Microfacet
{
public:
    RT_FORCE_INLINE Microfacet(float alpha)
        : mAlpha(alpha)
        , mAlphaSqr(alpha * alpha)
    { }

    float D(const math::Vector4& m) const
    {
        const float NdotH = m.z;
        const float cosThetaSq = math::Sqr(NdotH);
        const float tanThetaSq = math::Max(1.0f - cosThetaSq, 0.0f) / cosThetaSq;
        const float cosThetaQu = cosThetaSq * cosThetaSq;
        return mAlphaSqr * RT_INV_PI / (cosThetaQu * math::Sqr(mAlphaSqr + tanThetaSq));
    }

    RT_FORCE_INLINE float Pdf(const math::Vector4& m) const
    {
        return D(m) * math::Abs(m.z);
    }

    float G1(const float NdotX) const
    {
        float tanThetaSq = math::Max(1.0f - NdotX * NdotX, 0.0f) / (NdotX * NdotX);
        return 2.0f / (1.0f + math::Sqrt(1.0f + mAlphaSqr * tanThetaSq));
    }

    // shadowing-masking term
    float G(const float NdotV, const float NdotL) const
    {
        float tanThetaSqV = (1.0f - NdotV * NdotV) / (NdotV * NdotV);
        float tanThetaSqL = (1.0f - NdotL * NdotL) / (NdotL * NdotL);
        return 4.0f / ((1.0f + math::Sqrt(1.0f + mAlphaSqr * tanThetaSqV)) * (1.0f + math::Sqrt(1.0f + mAlphaSqr * tanThetaSqL)));
    }

    const math::Vector4 Sample(const math::Float2 u) const
    {
        // generate microfacet normal vector using GGX distribution function (Trowbridge-Reitz)
        const float cosThetaSqr = (1.0f - u.x) / (1.0f + (mAlphaSqr - 1.0f) * u.x);
        const float cosTheta = math::Sqrt(cosThetaSqr);
        const float sinTheta = math::Sqrt(1.0f - cosThetaSqr);
        const float phi = RT_2PI * u.y;

        return math::Vector4(sinTheta * math::Sin(phi), sinTheta * math::Cos(phi), cosTheta, 0.0f);
    }

private:
    float mAlpha;
    float mAlphaSqr;
};

} // namespace rt
