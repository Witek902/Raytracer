#include "PCH.h"

#include "PathTracer.h"
#include "PathTracerMIS.h"
#include "LightTracer.h"
#include "BidirectionalPathTracer.h"
#include "DebugRenderer.h"

namespace rt {

IRenderer::IRenderer(const Scene& scene)
    : mScene(scene)
{
}

IRenderer::~IRenderer() = default;

void IRenderer::Raytrace_Packet(RayPacket&, const Camera&, Film&, RenderingContext&) const
{
}

// TODO use reflection
IRenderer* CreateRenderer(const std::string& name, const Scene& scene)
{
    if (name == "Path Tracer")
    {
        return new PathTracer(scene);
    }
    else if (name == "Path Tracer MIS")
    {
        return new PathTracerMIS(scene);
    }
    else if (name == "Light Tracer")
    {
        return new LightTracer(scene);
    }
    else if (name == "Debug")
    {
        return new DebugRenderer(scene);
    }
    else if (name == "Bidirectional Path Tracer")
    {
        return new BidirectionalPathTracer(scene);
    }

    return nullptr;
}

} // namespace rt
