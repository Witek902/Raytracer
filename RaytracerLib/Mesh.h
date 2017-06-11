#pragma once

#include "RayLib.h"
#include "Math/Box.h"

#include "VertexBuffer.h"

namespace rt {


/**
 * Class representing a mesh.
 */
RT_ALIGN(16)
class RAYLIB_API Mesh
{
public:

private:
    // bounding box after scaling
    math::Box mBoundingBox;

    // vertex data
    VertexBuffer mVertexBuffer;

    // Scaling factor. Used when vertex data format is non-float
    float mScale;
};

} // namespace rt
