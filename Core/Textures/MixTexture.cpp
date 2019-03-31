#include "PCH.h"
#include "MixTexture.h"

namespace rt {

using namespace math;

MixTexture::MixTexture(const TexturePtr& textureA, const TexturePtr& textureB, const TexturePtr& textureMask)
    : mTextureA(textureA)
    , mTextureB(textureB)
    , mTextureMask(textureMask)
{
    RT_ASSERT(mTextureA);
    RT_ASSERT(mTextureB);
    RT_ASSERT(mTextureMask);
}

const char* MixTexture::GetName() const
{
    return "mix";
}

const Vector4 MixTexture::Evaluate(const Vector4& coords) const
{
    const Vector4 colorA = mTextureA->Evaluate(coords);
    const Vector4 colorB = mTextureB->Evaluate(coords);
    const Vector4 weight = mTextureMask->Evaluate(coords);

    return Vector4::Lerp(colorA, colorB, weight);
}

const Vector4 MixTexture::Sample(const Float2 u, Vector4& outCoords, float* outPdf) const
{
    // TODO
    
    outCoords = Vector4(u);

    if (outPdf)
    {
        *outPdf = 1.0f;
    }

    return MixTexture::Evaluate(outCoords);
}

} // namespace rt