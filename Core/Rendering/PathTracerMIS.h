#pragma once

#include "Renderer.h"
#include "../Material/BSDF/BSDF.h"

namespace rt {

struct ShadingData;
class ILight;

// Unidirectional path tracer
// Samples both BSDF and direct lighting
// Uses MIS (Multiple Importance Sampling)
class PathTracerMIS : public IRenderer
{
public:
    PathTracerMIS(const Scene& scene);

    virtual const char* GetName() const override;
    virtual const RayColor RenderPixel(const math::Ray& ray, const RenderParam& param, RenderingContext& ctx) const override;

    // for debugging
    math::Vector4 mLightSamplingWeight;
    math::Vector4 mBSDFSamplingWeight;

private:

    struct PathState
    {
        Uint32 depth = 0u;
        float lastPdfW = 1.0f;
        BSDF::EventType lastSampledBsdfEvent = BSDF::NullEvent;
        bool lastSpecular = true;
    };

    // importance sample light sources
    const RayColor SampleLights(const ShadingData& shadingData, const PathState& pathState, RenderingContext& context) const;

    // importance sample single light source
    const RayColor SampleLight(const ILight& light, const ShadingData& shadingData, const PathState& pathState, RenderingContext& context) const;

    // compute radiance from a hit local lights
    const RayColor EvaluateLight(const ILight& light, const math::Ray& ray, float dist, const PathState& pathState, RenderingContext& context) const;

    // compute radiance from global lights
    const RayColor EvaluateGlobalLights(const math::Ray& ray, const PathState& pathState, RenderingContext& context) const;
};

} // namespace rt
