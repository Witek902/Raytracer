#pragma once

#include "../RayLib.h"

#include <memory>

namespace rt {

// per-thread renderer-specific context
class IRendererContext : public NoCopyable
{
public:
    IRendererContext();
    virtual ~IRendererContext();
};

using RendererContextPtr = std::unique_ptr<IRendererContext>;

} // namespace rt
