#include "PCH.h"

#include "PathTracer.h"
#include "PathTracerMIS.h"
#include "LightTracer.h"
#include "VertexConnectionAndMerging.h"
#include "DebugRenderer.h"

namespace rt {

IRenderer::IRenderer(const Scene& scene)
    : mScene(scene)
{
}

IRenderer::~IRenderer() = default;

RendererContextPtr IRenderer::CreateContext() const
{
    return nullptr;
}

void IRenderer::PreRender(const Film&)
{
}

void IRenderer::PreRender(RenderingContext&)
{
}

void IRenderer::PreRenderPixel(const RenderParam&, RenderingContext&) const
{
}

void IRenderer::PreRenderGlobal(RenderingContext&)
{
}

void IRenderer::PreRenderGlobal()
{
}

void IRenderer::Raytrace_Packet(RayPacket&, const Camera&, Film&, RenderingContext&) const
{
}


// TODO use reflection
RendererPtr CreateRenderer(const std::string& name, const Scene& scene)
{
    if (name == "Path Tracer")
    {
        return RendererPtr(new PathTracer(scene));
    }
    else if (name == "Path Tracer MIS")
    {
        return RendererPtr(new PathTracerMIS(scene));
    }
    else if (name == "Light Tracer")
    {
        return RendererPtr(new LightTracer(scene));
    }
    else if (name == "Debug")
    {
        return RendererPtr(new DebugRenderer(scene));
    }
    else if (name == "VCM")
    {
        return RendererPtr(new VertexConnectionAndMerging(scene));
    }

    return nullptr;
}

} // namespace rt
