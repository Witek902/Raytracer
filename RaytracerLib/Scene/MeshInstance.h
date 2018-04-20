#pragma once

#include "../RayLib.h"
#include "../Math/Matrix.h"


namespace rt {

class Mesh;

/**
 * A mesh spawned on a scene.
 */
class RAYLIB_API MeshInstance
{
public:
    MeshInstance();

    const Mesh* mMesh;

    math::Matrix mTransform;
    math::Matrix mInvTransform;

    // TODO:
    // material modifiers
    // material mapping
};

} // namespace rt
