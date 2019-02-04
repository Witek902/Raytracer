#pragma once

#include "Renderer.h"
#include "../Material/BSDF/BSDF.h"

namespace rt {

struct ShadingData;
class ILight;

// Naive unidirectional path tracer
// Note: this renderer is unable to sample "delta" lights (point and directional lights) 
class PathTracer : public IRenderer
{
public:
    PathTracer(const Scene& scene);

    virtual const char* GetName() const override;
    virtual const RayColor TraceRay_Single(const math::Ray& ray, const Camera& camera, Film& film, RenderingContext& context) const override;

private:

    // compute radiance from a hit local lights
    const RayColor EvaluateLight(const ILight& light, const math::Ray& ray, Float dist, RenderingContext& context) const;

    // compute radiance from global lights
    const RayColor EvaluateGlobalLights(const math::Ray& ray, RenderingContext& context) const;
};

} // namespace rt
