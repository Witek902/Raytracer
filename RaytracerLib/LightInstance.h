#pragma once

#include "RayLib.h"
#include "Math/Matrix.h"


namespace rt {


class Light;


/**
 * A light spawned on a scene.
 */
class RAYLIB_API LightInstance
{
public:


private:
    math::Matrix mTransform;
    math::Matrix mInvTransform;
};

} // namespace rt
