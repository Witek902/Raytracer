#pragma once

#include "RayLib.h"
#include "VertexBufferDesc.h"
#include "Math/Box.h"
#include "Math/Ray.h"


namespace rt {

class Material;

struct MeshDesc
{
    VertexBufferDesc vertexBufferDesc;
    float scale;
    const char* debugName;

    MeshDesc()
        : scale(1.0f)
        , debugName(nullptr)
    { }
};


/**
 * Class representing a mesh.
 */
class RAYLIB_API IMesh
{
public:
    virtual ~IMesh() { }

    // Initialize the mesh
    virtual bool Initialize(const MeshDesc& desc) = 0;
};


} // namespace rt
