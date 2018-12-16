#pragma once

#include "VertexBufferDesc.h"

#include "../Math/Vector4.h"
#include "../Math/Triangle.h"
#include "../Math/Float3.h"


namespace rt {

namespace math {
class Triangle_Simd8;
}

class Material;

struct RT_ALIGN(16) VertexIndices
{
    Uint32 i0;
    Uint32 i1;
    Uint32 i2;
    Uint32 materialIndex;
};

struct RT_ALIGN(32) VertexShadingData
{
    math::Float3 normal;
    math::Float3 tangent;
    math::Float2 texCoord;
};


// Structure containing packed mesh data (vertices, vertex indices and material indices).
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
    void GetVertexIndices(const Uint32 triangleIndex, VertexIndices& indices) const;

    // get material for given a triangle
    const Material* GetMaterial(const Uint32 materialIndex) const;

    // extract preprocessed triangle data (for one triangle)
    const math::ProcessedTriangle& GetTriangle(const Uint32 triangleIndex) const;
    void GetTriangle(const Uint32 triangleIndex, math::Triangle_Simd8& outTriangle) const;

    void GetShadingData(const VertexIndices& indices, VertexShadingData& a, VertexShadingData& b, VertexShadingData& c) const;

    Uint32 GetNumVertices() const { return mNumVertices; }
    Uint32 GetNumTriangles() const { return mNumTriangles; }

private:

    char* mBuffer;
    math::ProcessedTriangle* mPreprocessedTriangles;

    size_t mVertexIndexBufferOffset;
    size_t mShadingDataBufferOffset;
    size_t mMaterialBufferOffset;

    Uint32 mNumVertices;
    Uint32 mNumTriangles;
    Uint32 mNumMaterials;

    std::vector<MaterialPtr> mMaterials;
};


} // namespace rt
