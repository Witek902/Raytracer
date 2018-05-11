#pragma once

#include "../RayLib.h"

namespace rt {

class Material;

/**
 * Structure describing a vertex buffer.
 */
struct VertexBufferDesc
{
    Uint32 numVertices;
    Uint32 numTriangles;
    Uint32 numMaterials;

    const Uint32* vertexIndexBuffer;
    const float* positions;
    const float* normals;
    const float* tangents;
    const float* texCoords;
    const Uint32* materialIndexBuffer;
    const Material** materials;

    VertexBufferDesc::VertexBufferDesc()
        : numVertices(0)
        , numTriangles(0)
        , numMaterials(0)
        , vertexIndexBuffer(nullptr)
        , positions(nullptr)
        , normals(nullptr)
        , tangents(nullptr)
        , texCoords(nullptr)
        , materials(nullptr)
        , materialIndexBuffer(nullptr)
    { }
};

} // namespace rt
