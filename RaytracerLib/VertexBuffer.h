#pragma once

#include "RayLib.h"
#include "Math/Vector.h"

namespace rt {

enum class VertexDataFormat : Uint8
{
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

/**
* Structure containing packed mesh data (vertices, vertex indices and material indices).
*/
class VertexBuffer
{
public:
    VertexBuffer();

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

    // data buffers
    void* mVertexIndexBuffer;
    void* mPositions;
    void* mNormals;
    void* mTangents;
    void* mTexCoords;
    void* mMaterialIndexBuffer;

    // data buffers formats
    VertexDataFormat mVertexIndexFormat;
    VertexDataFormat mPositionsFormat;
    VertexDataFormat mNormalsFormat;
    VertexDataFormat mTangentsFormat;
    VertexDataFormat mTexCoordsFormat;
    VertexDataFormat mMaterialIndexFormat;

    static void ExtractTriangleData2(const void* dataBuffer, VertexDataFormat format, const VertexIndices& indices, TriangleData& data);
    static void ExtractTriangleData3(const void* dataBuffer, VertexDataFormat format, const VertexIndices& indices, TriangleData& data);
};


} // namespace rt
