#pragma once

#include "../../RayLib.h"
#include "../../Utils/AlignmentAllocator.h"
#include "../../Math/Ray.h"
#include "../../Color/Color.h"

namespace rt {

class Material;

namespace math
{
    class Random;
}

struct SampledMaterialParameters
{
    Float roughness;
    Float IoR;
};

// Abstract class for Bidirectional Scattering Density Function
// Handles both reflection and transmission
// NOTE: all the calculations are performed in local-space of the hit point on a surface: X is tangent, Z is normal
class BSDF : public Aligned<16>
{
public:
    enum EventType : Uint8
    {
        NullEvent                   = 0,
        DiffuseReflectionEvent      = 1 << 0,
        DiffuseTransmissionEvent    = 1 << 1,
        GlossyReflectionEvent       = 1 << 2,
        GlossyRefractionEvent       = 1 << 3,
        SpecularReflectionEvent     = 1 << 4,
        SpecularRefractionEvent     = 1 << 5,

        DiffuseEvent                = DiffuseReflectionEvent | DiffuseTransmissionEvent,
        GlossyEvent                 = GlossyReflectionEvent | GlossyRefractionEvent,
        SpecularEvent               = SpecularReflectionEvent | SpecularRefractionEvent,

        ReflectiveEvent             = DiffuseReflectionEvent | GlossyReflectionEvent | SpecularReflectionEvent,
        TransmissiveEvent           = SpecularRefractionEvent | GlossyRefractionEvent | DiffuseTransmissionEvent,

        AnyEvent                    = ReflectiveEvent | TransmissiveEvent,
    };

    static constexpr Float CosEpsilon = 1.0e-5f;

    virtual ~BSDF() = default;

    struct SamplingContext
    {
        // inputs
        const Material& material;
        SampledMaterialParameters materialParam;
        const math::Vector4 outgoingDir;
        Wavelength& wavelength; // non-const, because can trigger dispersion
        math::Random& randomGenerator;

        // outputs
        Color outColor;
        math::Vector4 outIncomingDir;
        Float outPdf = 0.0f;
        EventType outEventType = NullEvent;
    };

    struct EvaluationContext
    {
        const Material& material;
        SampledMaterialParameters materialParam;
        const Wavelength& wavelength;
        const math::Vector4 outgoingDir;
        const math::Vector4 incomingDir;
    };

    // Importance sample the BSDF
    // Generates incoming light direction for given outgoing ray direction.
    // Returns ray weight (NdotL multiplied) as well as sampling probability of the generated direction.
    virtual bool Sample(SamplingContext& ctx) const = 0;

    // Evaluate BRDF
    // Optionally returns probability of sampling this direction
    // NOTE: the result is NdotL multiplied
    virtual const math::Vector4 Evaluate(const EvaluationContext& ctx, Float* outDirectPdfW = nullptr) const = 0;
};

} // namespace rt
