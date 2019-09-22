#pragma once

#include "../../RayLib.h"
#include "../../Rendering/ShadingData.h"
#include "../../Utils/Memory.h"
#include "../../Math/Ray.h"
#include "../../Color/RayColor.h"

namespace rt {

class Material;

namespace math
{
    class Random;
}

// Abstract class for Bidirectional Scattering Density Function
// Handles both reflection and transmission
// NOTE: all the calculations are performed in local-space of the hit point on a surface: X is tangent, Z is normal
// NOTE: it's not called "BRDF", because it handles transmission as well
class BSDF : public Aligned<16>
{
public:
    enum EventType : uint8
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

    enum PdfDirection
    {
        ForwardPdf,
        ReversePdf,
    };

    // If incoming/outgoing direction is at extremely grazing angle, the BSDF will early-return zero value
    // in order to avoid potential divisions by zero.
    static constexpr float CosEpsilon = 1.0e-5f;

    // If the roughness value of an material is below this treshold we fallback to perfectly specular event.
    // If we didn't do this, we would end up with an extremely high values of sampling PDF.
    static constexpr float SpecularEventRoughnessTreshold = 0.005f;

    virtual ~BSDF() = default;

    struct SamplingContext
    {
        // inputs
        const Material& material;
        SampledMaterialParameters materialParam;    // evaluated material parameters
        const math::Float3 sample;                  // random sample
        const math::Vector4 outgoingDir;            // fixed ray direction
        Wavelength& wavelength;                     // non-const, because can trigger dispersion

        // outputs
        RayColor outColor = RayColor::Zero();
        math::Vector4 outIncomingDir = math::Vector4::Zero();
        float outPdf = 0.0f;
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

    // get debug name
    virtual const char* GetName() const = 0;

    // Check if the distribution is Dirac delta
    // In other words it means that sampling the BSDF generates only specular events and evaluation returns zero.
    virtual bool IsDelta() const = 0;

    // Importance sample the BSDF
    // Generates incoming light direction for given outgoing ray direction.
    // Returns ray weight (NdotL multiplied) as well as sampling probability of the generated direction.
    virtual bool Sample(SamplingContext& ctx) const = 0;

    // Evaluate BSDF
    // Optionally returns probability of sampling this direction
    // NOTE: the result is NdotL multiplied
    virtual const RayColor Evaluate(const EvaluationContext& ctx, float* outDirectPdfW = nullptr, float* outReversePdfW = nullptr) const = 0;

    // Compute probability of scaterring event
    virtual float Pdf(const EvaluationContext& ctx, PdfDirection dir = ForwardPdf) const = 0;
};

} // namespace rt
