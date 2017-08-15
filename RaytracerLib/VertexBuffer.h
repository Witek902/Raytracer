#pragma once

#include "VertexBufferDesc.h"
#include "Math/Vector4.h"
#include "Math/Triangle.h"


namespace rt {

class Material;

struct VertexIndices
{
    Uint32 i0;
    Uint32 i1;
    Uint32 i2;
};


/**
* Structure containing packed mesh data (vertices, vertex indices and material indices).
*/
class VertexBuffer
{
public:
    VertexBuffer();
    ~VertexBuffer();

    // Clear buffers
    void Clear();

    // Initialize the vertex buffer with a new content
    bool Initialize(const VertexBufferDesc& desc);

    // get vertex indices for given triangle
    RT_FORCE_NOINLINE void GetVertexIndices(const Uint32 triangleIndex, VertexIndices& indices) const;

    // get material for given a triangle
    const Material* GetMaterial(const Uint32 triangleIndex) const;

    // extract vertex data (for one triangle)
    void GetVertexPositions(const VertexIndices& indices, math::Triangle& data) const;
    void GetVertexNormals(const VertexIndices& indices, math::Triangle& data) const;
    void GetVertexTangents(const VertexIndices& indices, math::Triangle& data) const;
    void GetVertexTexCoords(const VertexIndices& indices, math::Triangle& data) const;

    Uint32 GetNumVertices() const { return mNumVertices; }
    Uint32 GetNumTriangles() const { return mNumTriangles; }

private:
    // TODO keep in contiguous buffer + use offsets

    Uint32 mNumVertices;
    Uint32 mNumTriangles;

    char* mBuffer;

    Uint32 mVertexIndexBufferOffset;
    Uint32 mPositionsBufferOffset;
    Uint32 mNormalsBufferOffset;
    Uint32 mTangentsBufferOffset;
    Uint32 mTexCoordsBufferOffset;
    Uint32 mMaterialIndexBufferOffset;
    Uint32 mMaterialBufferOffset;

    // data buffers formats
    VertexDataFormat mVertexIndexFormat;
    VertexDataFormat mPositionsFormat;
    VertexDataFormat mNormalsFormat;
    VertexDataFormat mTangentsFormat;
    VertexDataFormat mTexCoordsFormat;
    VertexDataFormat mMaterialIndexFormat;

    static Uint32 GetElementSize(VertexDataFormat format);
    RT_FORCE_NOINLINE static void ExtractTriangleData2(const void* dataBuffer, VertexDataFormat format, const VertexIndices& indices, math::Triangle& data);
    RT_FORCE_NOINLINE static void ExtractTriangleData3(const void* dataBuffer, VertexDataFormat format, const VertexIndices& indices, math::Triangle& data);
};


} // namespace rt
