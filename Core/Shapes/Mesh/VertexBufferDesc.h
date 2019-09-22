#pragma once

#include "../../Math/Float3.h"

namespace rt {

class Material;
using MaterialPtr = std::shared_ptr<rt::Material>;

/**
 * Structure describing a vertex buffer.
 */
struct VertexBufferDesc
{
    uint32 numVertices = 0;
    uint32 numTriangles = 0;
    uint32 numMaterials = 0;

    const uint32* vertexIndexBuffer = nullptr;
    const math::Float3* positions = nullptr;
    const math::Float3* normals = nullptr;
    const math::Float3* tangents = nullptr;
    const math::Float2* texCoords = nullptr;
    const uint32* materialIndexBuffer = nullptr;
    const MaterialPtr* materials = nullptr;
};

} // namespace rt
