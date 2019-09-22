#pragma once

#include "VertexBufferDesc.h"

#include "../../Math/Vector4.h"
#include "../../Math/Triangle.h"
#include "../../Math/Float3.h"
#include "../../Containers/DynArray.h"

namespace rt {

namespace math {
class Triangle_Simd8;
}

class Material;

struct RT_ALIGN(16) VertexIndices
{
    uint32 i0;
    uint32 i1;
    uint32 i2;
    uint32 materialIndex;
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
    void GetVertexIndices(const uint32 triangleIndex, VertexIndices& indices) const;

    // get material for given a triangle
    const Material* GetMaterial(const uint32 materialIndex) const;

    // extract preprocessed triangle data (for one triangle)
    const math::ProcessedTriangle& GetTriangle(const uint32 triangleIndex) const;
    void GetTriangle(const uint32 triangleIndex, math::Triangle_Simd8& outTriangle) const;

    void GetShadingData(const VertexIndices& indices, VertexShadingData& a, VertexShadingData& b, VertexShadingData& c) const;

    RT_FORCE_INLINE uint32 GetNumVertices() const { return mNumVertices; }
    RT_FORCE_INLINE uint32 GetNumTriangles() const { return mNumTriangles; }

private:

    char* mBuffer;
    math::ProcessedTriangle* mPreprocessedTriangles;

    size_t mVertexIndexBufferOffset;
    size_t mShadingDataBufferOffset;
    size_t mMaterialBufferOffset;

    uint32 mNumVertices;
    uint32 mNumTriangles;

    DynArray<MaterialPtr> mMaterials;
};


} // namespace rt
