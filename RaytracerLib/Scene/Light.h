#pragma once

#include "../RayLib.h"
#include "../Math/Vector4.h"


namespace rt {


enum class LightType
{
    Omni,
    Spot,
    Dir,
    Area,
    Volume,
};


/**
 * Collection of light parameters.
 */
class RAYLIB_API Light
{
public:

private:
    LightType mType;

    math::Vector4 mColor;
};


} // namespace rt
