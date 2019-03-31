#include "PCH.h"
#include "Texture.h"

namespace rt {

ITexture::~ITexture() = default;

bool ITexture::MakeSamplable()
{
    return true;
}

bool ITexture::IsSamplable() const
{
    return true;
}

} // namespace rt