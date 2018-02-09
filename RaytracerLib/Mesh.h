#pragma once

#include "RayLib.h"
#include "VertexBufferDesc.h"
#include "Math/Box.h"
#include "Math/Ray.h"

#include <string>


namespace rt {

class Material;

struct MeshDesc
{
    VertexBufferDesc vertexBufferDesc;
    std::string path;
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
