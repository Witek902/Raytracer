#include "PCH.h"
#include "Renderer.h"

namespace rt {

IRenderer::IRenderer(const Scene& scene)
    : mScene(scene)
{
}

IRenderer::~IRenderer() = default;

} // namespace rt
