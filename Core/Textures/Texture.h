#pragma once

#include "../RayLib.h"
#include "../Math/Vector4.h"

#include <memory>

namespace rt {

/**
 * Class representing 2D texture.
 */
class ITexture
{
public:
    RAYLIB_API virtual ~ITexture();

    // get human-readable description
    virtual const char* GetName() const = 0;

    // evaluate texture color at given coordinates
    virtual const math::Vector4 Evaluate(const math::Vector4& coords) const = 0;

    // generate random sample on the texture
    virtual const math::Vector4 Sample(const math::Float2 u, math::Vector4& outCoords, float* outPdf = nullptr) const = 0;

    // must be called before using Sample() method
    virtual bool MakeSamplable();

    // check if the texture is samplable (if it's not, calling Sample is illegal)
    virtual bool IsSamplable() const;
};

using TexturePtr = std::shared_ptr<ITexture>;

} // namespace rt
