#pragma once

#include "../RayLib.h"
#include "../Math/Vector4.h"

#include <memory>

namespace rt {

enum class TextureAddressMode : Uint8
{
    Repeat = 0,
    Clamp = 1,
    Border = 2,
};

enum class TextureFilterMode : Uint8
{
    NearestNeighbor = 0,
    Bilinear = 1,
};

struct SamplerDesc
{
    math::Vector4 borderColor = math::Vector4::Zero();
    TextureAddressMode addressU = TextureAddressMode::Repeat;
    TextureAddressMode addressV = TextureAddressMode::Repeat;
    TextureFilterMode filter = TextureFilterMode::Bilinear;
    bool forceLinearSpace = false;
};

/**
 * Class representing 2D texture.
 */
class RAYLIB_API ITexture
{
public:
    virtual ~ITexture() { }

    // evaluate texture color at given coordinates
    virtual const math::Vector4 Evaluate(math::Vector4 coords, const SamplerDesc& sampler = SamplerDesc()) const = 0;
};

using TexturePtr = std::shared_ptr<ITexture>;

} // namespace rt
