#pragma once

#include "../RayLib.h"
#include "../Utils/AlignmentAllocator.h"
#include "../Math/Ray.h"

namespace rt {

namespace math
{
class Random;
}

// Abstract class for Bidirectional Scattering Density Function
// Handles both reflection and transmission
// NOTE: all the calculations are performed in local-space of the hit point on a surface: X is tangent, Z is normal
class BSDF : public Aligned<16>
{
public:
    virtual ~BSDF() = default;

    // Importance sample the BSDF
    // Generates incoming light direction for given outgoing ray direction
    // Ray weight (BSDF+PDF correction) is returned as well.
    // TODO wavelength
    virtual void Sample(const math::Vector4& outgoingDir, math::Vector4& outIncomingDir, math::Vector4& outWeight, math::Random& randomGenerator) const = 0;

    // Evaluate BRDF
    // TODO wavelength
    virtual math::Vector4 Evaluate(const math::Vector4& outgoingDir, const math::Vector4& incomingDir) const = 0;
};

///

// diffuse reflection BRDF
class OrenNayarBSDF : public BSDF
{
public:
    OrenNayarBSDF();
    explicit OrenNayarBSDF(const math::Vector4& baseColor, const float roughness = 0.1f);

    virtual void Sample(const math::Vector4& outgoingDir, math::Vector4& outIncomingDir, math::Vector4& outWeight, math::Random& randomGenerator) const override;
    virtual math::Vector4 Evaluate(const math::Vector4& outgoingDir, const math::Vector4& incomingDir) const override;

    math::Vector4 mBaseColor;
    float mRoughness;
};

// Cook-Torrance specular BSDF
class CookTorranceBSDF : public BSDF
{
public:
    explicit CookTorranceBSDF(const float roughness = 0.1f);

    virtual void Sample(const math::Vector4& outgoingDir, math::Vector4& outIncomingDir, math::Vector4& outWeight, math::Random& randomGenerator) const override;
    virtual math::Vector4 Evaluate(const math::Vector4& outgoingDir, const math::Vector4& incomingDir) const override;

    // get normal distribution factor (D coefficient)
    float NormalDistribution(const float NdotH) const;

    float mRougness;
};

// TODO general Cook-Torrance specular reflection/refraction BSDF
class TransparencyBSDF : public BSDF
{
public:
    TransparencyBSDF(float ior);
    virtual void Sample(const math::Vector4& outgoingDir, math::Vector4& outIncomingDir, math::Vector4& outWeight, math::Random& randomGenerator) const override;
    virtual math::Vector4 Evaluate(const math::Vector4& outgoingDir, const math::Vector4& incomingDir) const override;

    float IOR;
};

} // namespace rt
