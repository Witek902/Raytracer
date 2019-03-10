#pragma once

#include "../RayLib.h"

namespace rt {

enum class TextureFilterMode : Uint8
{
    NearestNeighbor = 0,
    Bilinear = 1,
};

struct TextureEvaluator
{
    TextureFilterMode filter = TextureFilterMode::NearestNeighbor;
    bool forceLinearSpace = false;
};

} // namespace rt
