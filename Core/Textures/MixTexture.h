#pragma once

#include "Texture.h"

namespace rt {

class MixTexture : public ITexture
{
public:
    RAYLIB_API MixTexture(const TexturePtr& textureA, const TexturePtr& textureB, const TexturePtr& textureMask);

    virtual const char* GetName() const override;
    virtual const math::Vector4 Evaluate(const math::Vector4& coords) const override;
    virtual const math::Vector4 Sample(const math::Float2 u, math::Vector4& outCoords, float* outPdf) const override;

private:
    TexturePtr mTextureA;
    TexturePtr mTextureB;
    TexturePtr mTextureMask;
};

} // namespace rt
