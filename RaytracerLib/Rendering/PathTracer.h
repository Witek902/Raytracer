#pragma once

#include "Renderer.h"

namespace rt {

struct ShadingData;

// Unidirectional path tracer
class RAYLIB_API PathTracer : public IRenderer
{
public:
    PathTracer(const Scene& scene);

    virtual const Color TraceRay_Single(const math::Ray& ray, RenderingContext& context) const override;

    // a.k.a. next event estimation (NEE)
    bool mSampleLights = true;

private:
    // importance sample light sources
    const Color SampleLights(const math::Ray& ray, const ShadingData& shadingData, RenderingContext& context) const;
};

} // namespace rt
