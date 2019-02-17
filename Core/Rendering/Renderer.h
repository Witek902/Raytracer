#pragma once

#include "RendererContext.h"

#include "../Color/RayColor.h"
#include "../Math/Ray.h"
#include "../Utils/AlignmentAllocator.h"

namespace rt {

class Film;
class Scene;
class Camera;
struct RenderingContext;
struct RayPacket;

// abstract scene rendering interface
class RT_ALIGN(16) IRenderer : public Aligned<16>
{
public:
    struct RenderParam
    {
        Uint32 pixelIndex;
        const Camera& camera;
        Film& film;
    };

    IRenderer(const Scene& scene);

    RAYLIB_API virtual ~IRenderer();

    // TODO batch & multisample rendering
    // TODO cancelation of ongoing rendering

    virtual const char* GetName() const = 0;

    // create per-thread context
    virtual RendererContextPtr CreateContext() const;

    // TODO clean this up...
    // idea: each renderer should report what passes it requires, etc.

    // optional rendering pre-pass, called once for every thread
    virtual void PreRender(const Film& film);
    virtual void PreRender(RenderingContext& ctx);

    // optional rendering pre-pass, called for every pixel on screen
    // Note: this will be called from multiple threads, each thread provides own RenderingContext
    virtual void PreRenderPixel(const RenderParam& param, RenderingContext& ctx) const;

    // optional rendering pre-pass, called once (single threaded)
    virtual void PreRenderGlobal(RenderingContext& ctx);
    virtual void PreRenderGlobal();

    // called for every pixel on screen during rendering
    // Note: this will be called from multiple threads, each thread provides own RenderingContext
    virtual const RayColor RenderPixel(const math::Ray& ray, const RenderParam& param, RenderingContext& ctx) const = 0;


    virtual void Raytrace_Packet(RayPacket& packet, const Camera& camera, Film& film, RenderingContext& context) const;

protected:
    const Scene& mScene;

private:
    IRenderer(const IRenderer&) = delete;
    IRenderer& operator = (const IRenderer&) = delete;
    IRenderer(IRenderer&&) = delete;
    IRenderer& operator = (IRenderer&&) = delete;
};

using RendererPtr = std::shared_ptr<IRenderer>;

RAYLIB_API RendererPtr CreateRenderer(const std::string& name, const Scene& scene);

} // namespace rt
