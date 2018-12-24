#pragma once

#include "../RayLib.h"

namespace rt {

class Material;
using MaterialPtr = std::shared_ptr<rt::Material>;

/**
 * Structure describing a vertex buffer.
 */
struct VertexBufferDesc
{
    Uint32 numVertices = 0;
    Uint32 numTriangles = 0;
    Uint32 numMaterials = 0;

    const Uint32* vertexIndexBuffer = nullptr;
    const float* positions = nullptr;
    const float* normals = nullptr;
    const float* tangents = nullptr;
    const float* texCoords = nullptr;
    const Uint32* materialIndexBuffer = nullptr;
    const MaterialPtr* materials = nullptr;
};

} // namespace rt
