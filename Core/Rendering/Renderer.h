#pragma once

#include "../RayLib.h"

#include "../Color/RayColor.h"
#include "../Math/Ray.h"

namespace rt {

class Film;
class Scene;
class Camera;
struct RenderingContext;
struct RayPacket;


// abstract scene rendering interface
class IRenderer
{
public:
    IRenderer(const Scene& scene);

    RAYLIB_API virtual ~IRenderer();

    // TODO batch & multisample rendering
    // TODO cancelation of ongoing rendering

    virtual const char* GetName() const = 0;

    virtual const RayColor TraceRay_Single(const math::Ray& ray, const Camera& camera, Film& film, RenderingContext& context) const = 0;

    virtual void Raytrace_Packet(RayPacket& packet, const Camera& camera, Film& film, RenderingContext& context) const;

protected:
    const Scene& mScene;

private:
    IRenderer(const IRenderer&) = delete;
    IRenderer& operator = (const IRenderer&) = delete;
    IRenderer(IRenderer&&) = delete;
    IRenderer& operator = (IRenderer&&) = delete;
};

RAYLIB_API IRenderer* CreateRenderer(const std::string& name, const Scene& scene);

} // namespace rt
