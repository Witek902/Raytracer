#include "PCH.h"
#include "VertexBuffer.h"
#include "Utils/Logger.h"
#include "Utils/AlignmentAllocator.h"
#include "Math/Simd8Triangle.h"

#include <string.h>

namespace rt {

static_assert(sizeof(VertexIndices) == 16, "Invalid size");
static_assert(sizeof(VertexShadingData) == 32, "Invalid size");

using namespace math;


VertexBuffer::VertexBuffer()
    : mBuffer(nullptr)
    , mPreprocessedTriangles(nullptr)
{
    Clear();
}

VertexBuffer::~VertexBuffer()
{
    Clear();
}

void VertexBuffer::Clear()
{
    if (mBuffer)
    {
        AlignedFree(mBuffer);
        mBuffer = nullptr;
    }

    if (mPreprocessedTriangles)
    {
        AlignedFree(mPreprocessedTriangles);
        mPreprocessedTriangles = nullptr;
    }

    mNumVertices = 0;
    mNumTriangles = 0;
    mVertexIndexBufferOffset = 0;
    mShadingDataBufferOffset = 0;
    mMaterialBufferOffset = 0;
}

bool VertexBuffer::Initialize(const VertexBufferDesc& desc)
{
    Clear();

    if (desc.numTriangles == 0)
    {
        RT_LOG_WARNING("Creating empty vertex buffer");
        return true;
    }

    if (desc.numVertices > 3 * desc.numTriangles)
    {
        RT_LOG_WARNING("There are redundant (unused) vertices");
    }

    if (!desc.positions)
    {
        RT_LOG_ERROR("Positions buffer must be provided");
        return false;
    }

    if (!desc.vertexIndexBuffer)
    {
        RT_LOG_ERROR("Index buffer must be provided");
        return false;
    }

    const size_t preprocessedTrianglesBufferSize = sizeof(ProcessedTriangle) * desc.numTriangles;
    const size_t positionsBufferSize = sizeof(Float3) * desc.numVertices;
    const size_t indexBufferSize = sizeof(VertexIndices) * desc.numTriangles;
    const size_t shadingDataBufferSize = sizeof(VertexShadingData) * desc.numVertices;
    const size_t materialBufferSize = sizeof(Material*) * desc.numMaterials;

    mVertexIndexBufferOffset = RoundUp<size_t>(positionsBufferSize, sizeof(VertexIndices));
    mShadingDataBufferOffset = RoundUp<size_t>(mVertexIndexBufferOffset + indexBufferSize, sizeof(VertexShadingData));
    mMaterialBufferOffset = mShadingDataBufferOffset + shadingDataBufferSize;

    const size_t bufferSizeRequired = mMaterialBufferOffset + materialBufferSize;


    RT_LOG_DEBUG("Allocating vertex buffer for mesh, size = %u", bufferSizeRequired);
    mBuffer = (char*)AlignedMalloc(bufferSizeRequired, RT_CACHE_LINE_SIZE);
    if (!mBuffer)
    {
        RT_LOG_ERROR("Memory allocation failed");
        return false;
    }

    // preprocess triangles
    {
        mPreprocessedTriangles = (ProcessedTriangle*)AlignedMalloc(preprocessedTrianglesBufferSize, RT_CACHE_LINE_SIZE);
        if (!mPreprocessedTriangles)
        {
            RT_LOG_ERROR("Memory allocation failed");
            return false;
        }

        const Float3* positions = (const Float3*)desc.positions;
        const Uint32* indexBuffer = desc.vertexIndexBuffer;

        for (Uint32 i = 0; i < desc.numTriangles; ++i)
        {
            const Vector4 v0(positions[indexBuffer[3 * i + 0]]);
            const Vector4 v1(positions[indexBuffer[3 * i + 1]]);
            const Vector4 v2(positions[indexBuffer[3 * i + 2]]);

            mPreprocessedTriangles[i].v0 = v0.ToFloat3();
            mPreprocessedTriangles[i].edge1 = (v1 - v0).ToFloat3();
            mPreprocessedTriangles[i].edge2 = (v2 - v0).ToFloat3();
        }
    }

    // fill index buffer
    {
        VertexIndices* buffer = reinterpret_cast<VertexIndices*>(mBuffer + mVertexIndexBufferOffset);
        for (Uint32 i = 0; i < desc.numTriangles; ++i)
        {
            buffer[i].i0 = desc.vertexIndexBuffer[3 * i];
            buffer[i].i1 = desc.vertexIndexBuffer[3 * i + 1];
            buffer[i].i2 = desc.vertexIndexBuffer[3 * i + 2];
            buffer[i].materialIndex = desc.materialIndexBuffer[i];
        }
    }

    memcpy(mBuffer, desc.positions, positionsBufferSize);

    // fill vertex shading data buffer
    {
        VertexShadingData* buffer = reinterpret_cast<VertexShadingData*>(mBuffer + mShadingDataBufferOffset);
        for (Uint32 i = 0; i < desc.numVertices; ++i)
        {
            buffer[i].normal = desc.normals ? Float3(desc.normals + 3 * i) : Float3();
            buffer[i].tangent = desc.tangents ? Float3(desc.tangents + 3 * i) : Float3();
            buffer[i].texCoord = desc.texCoords ? Float2(desc.texCoords + 2 * i) : Float2();
        }
    }

    if (materialBufferSize > 0)
    {
        memcpy(mBuffer + mMaterialBufferOffset, desc.materials, materialBufferSize);
    }

    mNumVertices = desc.numVertices;
    mNumTriangles = desc.numTriangles;
    mNumMaterials = desc.numMaterials;

    return true;
}

void VertexBuffer::GetVertexIndices(const Uint32 triangleIndex, VertexIndices& indices) const
{
    RT_ASSERT(triangleIndex < mNumTriangles);

    const VertexIndices* buffer = reinterpret_cast<const VertexIndices*>(mBuffer + mVertexIndexBufferOffset);
    indices = buffer[triangleIndex];
}

const Material* VertexBuffer::GetMaterial(const Uint32 materialIndex) const
{
    RT_ASSERT(materialIndex < mNumMaterials);

    const Material** materialBufferData = reinterpret_cast<const Material**>(mBuffer + mMaterialBufferOffset);
    return materialBufferData[materialIndex];
}

void VertexBuffer::ExtractTriangleData3(const void* dataBuffer, const VertexIndices& indices, math::Triangle& data)
{
    const Float* typedDataBuffer = reinterpret_cast<const Float*>(dataBuffer);
    const Float* v0 = typedDataBuffer + 3u * indices.i0;
    const Float* v1 = typedDataBuffer + 3u * indices.i1;
    const Float* v2 = typedDataBuffer + 3u * indices.i2;

    // clear W component (there can be garbage)
    const Vector4 mask = VECTOR_MASK_XYZ;
    data.v0 = Vector4(v0) & mask;
    data.v1 = Vector4(v1) & mask;
    data.v2 = Vector4(v2) & mask;
}

ProcessedTriangle VertexBuffer::GetTriangle(const Uint32 triangleIndex) const
{
    return mPreprocessedTriangles[triangleIndex];
}

void VertexBuffer::GetTriangle(const Uint32 triangleIndex, Triangle_Simd8& outTriangle) const
{
    const ProcessedTriangle& tri = mPreprocessedTriangles[triangleIndex];
    outTriangle.v0 = Vector3x8(tri.v0);
    outTriangle.edge1 = Vector3x8(tri.edge1);
    outTriangle.edge2 = Vector3x8(tri.edge2);
}

void VertexBuffer::GetShadingData(const VertexIndices& indices, VertexShadingData& a, VertexShadingData& b, VertexShadingData& c) const
{
    const VertexShadingData* buffer = reinterpret_cast<const VertexShadingData*>(mBuffer + mShadingDataBufferOffset);
    a = buffer[indices.i0];
    b = buffer[indices.i1];
    c = buffer[indices.i2];
}

} // namespace rt
