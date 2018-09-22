#pragma once

#include "../RayLib.h"
#include "../Utils/AlignmentAllocator.h"
#include "../Math/Ray.h"

namespace rt {

class Material;
struct Wavelength;
struct Color;

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
    virtual void Sample(Wavelength& wavelength, const math::Vector4& outgoingDir, math::Vector4& outIncomingDir, Color& outWeight, math::Random& randomGenerator) const = 0;

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

    virtual void Sample(Wavelength& wavelength, const math::Vector4& outgoingDir, math::Vector4& outIncomingDir, Color& outWeight, math::Random& randomGenerator) const override;
    virtual math::Vector4 Evaluate(const math::Vector4& outgoingDir, const math::Vector4& incomingDir) const override;

    math::Vector4 mBaseColor;
    float mRoughness;
};

// Cook-Torrance specular BSDF
class CookTorranceBSDF : public BSDF
{
public:
    explicit CookTorranceBSDF(const float roughness = 0.1f);

    virtual void Sample(Wavelength& wavelength, const math::Vector4& outgoingDir, math::Vector4& outIncomingDir, Color& outWeight, math::Random& randomGenerator) const override;
    virtual math::Vector4 Evaluate(const math::Vector4& outgoingDir, const math::Vector4& incomingDir) const override;

    float mRougness;
};

// TODO general Cook-Torrance specular reflection/refraction BSDF
class TransparencyBSDF : public BSDF
{
public:
    TransparencyBSDF(const Material& material);
    virtual void Sample(Wavelength& wavelength, const math::Vector4& outgoingDir, math::Vector4& outIncomingDir, Color& outWeight, math::Random& randomGenerator) const override;
    virtual math::Vector4 Evaluate(const math::Vector4& outgoingDir, const math::Vector4& incomingDir) const override;

    // HACK
    const Material& material;
};

} // namespace rt
