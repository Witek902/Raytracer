#pragma once

#include "../RayLib.h"
#include "../Math/Float3.h"

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
    const math::Float3* positions = nullptr;
    const math::Float3* normals = nullptr;
    const math::Float3* tangents = nullptr;
    const math::Float2* texCoords = nullptr;
    const Uint32* materialIndexBuffer = nullptr;
    const MaterialPtr* materials = nullptr;
};

} // namespace rt
