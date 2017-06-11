#pragma once

#include "RayLib.h"
#include "Math/Vector.h"

namespace rt {

enum class VertexDataFormat : Uint8
{
    None,
    Float,
    HalfFloat,
    Int32,
    Int16,
    Int8,
};


struct VertexIndices
{
    Uint32 i0;
    Uint32 i1;
    Uint32 i2;
};

// Tuple of triangle data
// Can be positions, normals, tangents of texture coords
struct RT_ALIGN(16) TriangleData
{
    math::Vector d0;
    math::Vector d1;
    math::Vector d2;
};

struct VertexBufferDesc
{
    Uint32 numVertices;
    Uint32 numIndices;

    void* vertexIndexBuffer;
    void* positions;
    void* normals;
    void* tangents;
    void* texCoords;
    void* materialIndexBuffer;

    VertexDataFormat vertexIndexFormat;
    VertexDataFormat positionsFormat;
    VertexDataFormat normalsFormat;
    VertexDataFormat tangentsFormat;
    VertexDataFormat texCoordsFormat;
    VertexDataFormat materialIndexFormat;
};

/**
* Structure containing packed mesh data (vertices, vertex indices and material indices).
*/
class RAYLIB_API VertexBuffer
{
public:
    VertexBuffer();
    ~VertexBuffer();

    // Clear buffers
    void Clear();

    // Initialize the vertex buffer with a new content
    bool Initialize(const VertexBufferDesc& desc);

    // get vertex indices for given triangle
    void GetVertexIndices(const Uint32 triangleIndex, VertexIndices& indices) const;

    // get material index for given triangle
    Uint32 GetMaterialIndex(const Uint32 triangleIndex) const;

    // extract vertex data (for one triangle)
    void GetVertexPositions(const VertexIndices& indices, TriangleData& data) const;
    void GetVertexNormals(const VertexIndices& indices, TriangleData& data) const;
    void GetVertexTangents(const VertexIndices& indices, TriangleData& data) const;
    void GetVertexTexCoords(const VertexIndices& indices, TriangleData& data) const;

private:
    // TODO keep in contiguous buffer + use offsets

    Uint32 mNumVertices;
    Uint32 mNumIndices;

    char* mBuffer;

    Uint32 mVertexIndexBufferOffset;
    Uint32 mPositionsBufferOffset;
    Uint32 mNormalsBufferOffset;
    Uint32 mTangentsBufferOffset;
    Uint32 mTexCoordsBufferOffset;
    Uint32 mMaterialIndexBufferOffset;

    // data buffers formats
    VertexDataFormat mVertexIndexFormat;
    VertexDataFormat mPositionsFormat;
    VertexDataFormat mNormalsFormat;
    VertexDataFormat mTangentsFormat;
    VertexDataFormat mTexCoordsFormat;
    VertexDataFormat mMaterialIndexFormat;

    static Uint32 GetElementSize(VertexDataFormat format);
    static void ExtractTriangleData2(const void* dataBuffer, VertexDataFormat format, const VertexIndices& indices, TriangleData& data);
    static void ExtractTriangleData3(const void* dataBuffer, VertexDataFormat format, const VertexIndices& indices, TriangleData& data);
};


} // namespace rt
