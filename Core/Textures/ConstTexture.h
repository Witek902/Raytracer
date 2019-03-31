#pragma once

#include "Texture.h"
#include "../Utils/AlignmentAllocator.h"

namespace rt {

// constant color texture
class RT_ALIGN(16) ConstTexture
    : public ITexture
    , public Aligned<16>
{
public:
    RAYLIB_API ConstTexture(const math::Vector4& color);

    virtual const char* GetName() const override;
    virtual const math::Vector4 Evaluate(const math::Vector4& coords) const override;
    virtual const math::Vector4 Sample(const math::Float2 u, math::Vector4& outCoords, float* outPdf) const override;

private:
    math::Vector4 mColor;
};

} // namespace rt
