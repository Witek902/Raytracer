#include "PCH.h"
#include "Renderer.h"

namespace rt {

IRenderer::IRenderer(const Scene& scene)
    : mScene(scene)
{
}

IRenderer::~IRenderer() = default;

void IRenderer::Raytrace_Packet(RayPacket&, RenderingContext&, Viewport&) const
{
}

} // namespace rt
