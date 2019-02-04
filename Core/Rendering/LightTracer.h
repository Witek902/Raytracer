#pragma once

#include "Renderer.h"
#include "../Material/BSDF/BSDF.h"

namespace rt {

struct ShadingData;
class ILight;

// Naive unidirectional light tracer
// Traces random light paths starting from light surface
// After hitting a geometry, connects to camera and splats the contribution onto film
// Note: This renderer is unable to render specular materials viewed by the camera directly
class LightTracer : public IRenderer
{
public:
    LightTracer(const Scene& scene);

    virtual const char* GetName() const override;
    virtual const RayColor TraceRay_Single(const math::Ray& ray, const Camera& camera, Film& film, RenderingContext& context) const override;

private:

};

} // namespace rt
