#pragma once

#include "Texture.h"
#include "../Utils/AlignmentAllocator.h"

namespace rt {

// 2D simplex noise texture
class RT_ALIGN(16) NoiseTexture
    : public ITexture
    , public Aligned<16>
{
public:
    RAYLIB_API NoiseTexture(const math::Vector4& colorA, const math::Vector4& colorB, const Uint32 numOctaves = 1);

    virtual const char* GetName() const override;
    virtual const math::Vector4 Evaluate(const math::Vector4& coords) const override;
    virtual const math::Vector4 Sample(const math::Float2 u, math::Vector4& outCoords, float* outPdf) const override;

    float EvaluateInternal(const math::Vector4& coords) const;

private:
    math::Vector4 mColorA;
    math::Vector4 mColorB;
    Uint32 mNumOctaves;
};

} // namespace rt
