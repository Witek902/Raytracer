#pragma once

#include "../RayLib.h"

#include "../Color/Color.h"
#include "../Math/Ray.h"

namespace rt {

class Scene;
struct RenderingContext;

// abstract scene rendering interface
class RAYLIB_API IRenderer
{
public:
    IRenderer(const Scene& scene);

    virtual ~IRenderer();

    // TODO batch & multisample rendering
    // TODO cancelation of ongoing rendering

    virtual const Color TraceRay_Single(const math::Ray& ray, RenderingContext& context) const = 0;

protected:
    const Scene& mScene;

private:
    IRenderer(IRenderer&&) = delete;
    IRenderer& operator = (IRenderer&&) = delete;
};

} // namespace rt
