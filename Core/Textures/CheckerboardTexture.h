#pragma once

#include "Texture.h"
#include "../Utils/Memory.h"

namespace rt {

class RT_ALIGN(16) CheckerboardTexture
    : public ITexture
    , public Aligned<16>
{
public:
    RAYLIB_API CheckerboardTexture(const math::Vector4& colorA, math::Vector4& colorB);

    virtual const char* GetName() const override;
    virtual const math::Vector4 Evaluate(const math::Vector4& coords) const override;
    virtual const math::Vector4 Sample(const math::Float2 u, math::Vector4& outCoords, float* outPdf) const override;

private:
    math::Vector4 mColorA;
    math::Vector4 mColorB;

    // probability of sampling color A
    float mPdf;
};

} // namespace rt
