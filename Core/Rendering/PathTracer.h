#pragma once

#include "Renderer.h"
#include "../Material/BSDF/BSDF.h"

namespace rt {

struct ShadingData;
class ILight;

// Unidirectional path tracer
class RAYLIB_API PathTracer : public IRenderer
{
public:
    PathTracer(const Scene& scene);

    virtual const RayColor TraceRay_Single(const math::Ray& ray, RenderingContext& context) const override;

    // a.k.a. next event estimation (NEE)
    bool mSampleLights;

private:

    struct PathState
    {
        Uint32 depth = 0u;
        Float lastPdfW = 1.0f;
        BSDF::EventType lastSampledBsdfEvent = BSDF::NullEvent;
        bool lastSpecular = true;
    };

    // importance sample light sources
    const RayColor SampleLights(const ShadingData& shadingData, const PathState& pathState, RenderingContext& context) const;

    // importance sample single light source
    const RayColor SampleLight(const ILight& light, const ShadingData& shadingData, const PathState& pathState, RenderingContext& context) const;

    // compute radiance from a hit local lights
    const RayColor EvaluateLight(const ILight& light, const math::Ray& ray, Float dist, const PathState& pathState, RenderingContext& context) const;

    // compute radiance from global lights
    const RayColor EvaluateGlobalLights(const math::Ray& ray, const PathState& pathState, RenderingContext& context) const;
};

} // namespace rt
