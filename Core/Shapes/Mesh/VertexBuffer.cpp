#include "PCH.h"
#include "VertexBuffer.h"
#include "Utils/Logger.h"
#include "Utils/AlignmentAllocator.h"
#include "Math/Simd8Triangle.h"


namespace rt {

static_assert(sizeof(VertexIndices) == 16, "Invalid size");
static_assert(sizeof(VertexShadingData) == 32, "Invalid size");
static_assert(alignof(VertexIndices) == 16, "Invalid alignment");
static_assert(alignof(VertexShadingData) == 32, "Invalid alignment");

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

    mMaterials.Clear();
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

    mVertexIndexBufferOffset = RoundUp<size_t>(positionsBufferSize, alignof(VertexIndices));
    mShadingDataBufferOffset = RoundUp<size_t>(mVertexIndexBufferOffset + indexBufferSize, alignof(VertexShadingData));
    mMaterialBufferOffset = mShadingDataBufferOffset + shadingDataBufferSize;

    const size_t bufferSizeRequired = mMaterialBufferOffset + materialBufferSize;


    RT_LOG_DEBUG("Allocating vertex buffer for mesh, size = %u", bufferSizeRequired);
    mBuffer = (char*)AlignedMalloc(bufferSizeRequired, RT_CACHE_LINE_SIZE);
    if (!mBuffer)
    {
        RT_LOG_ERROR("Memory allocation failed");
        return false;
    }

    // validate vertices
    {
        for (Uint32 i = 0; i < desc.numVertices; ++i)
        {
            RT_ASSERT(desc.positions[i].IsValid(), "Corrupted vertex position");
        }
    }

    // preprocess triangles
    {
        mPreprocessedTriangles = (ProcessedTriangle*)AlignedMalloc(preprocessedTrianglesBufferSize, RT_CACHE_LINE_SIZE);
        if (!mPreprocessedTriangles)
        {
            RT_LOG_ERROR("Memory allocation failed");
            return false;
        }

        const Float3* positions = desc.positions;
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
            VertexIndices& indices = buffer[i];

            indices.i0 = desc.vertexIndexBuffer[3 * i];
            indices.i1 = desc.vertexIndexBuffer[3 * i + 1];
            indices.i2 = desc.vertexIndexBuffer[3 * i + 2];
            indices.materialIndex = desc.materialIndexBuffer[i];

            RT_ASSERT(indices.i0 < desc.numVertices, "Vertex index out of bounds");
            RT_ASSERT(indices.i1 < desc.numVertices, "Vertex index out of bounds");
            RT_ASSERT(indices.i2 < desc.numVertices, "Vertex index out of bounds");
            RT_ASSERT(indices.materialIndex < desc.numMaterials || indices.materialIndex == UINT32_MAX, "Material index out of bounds");
        }
    }

    memcpy(mBuffer, desc.positions, positionsBufferSize);

    // fill vertex shading data buffer
    {
        VertexShadingData* buffer = reinterpret_cast<VertexShadingData*>(mBuffer + mShadingDataBufferOffset);
        for (Uint32 i = 0; i < desc.numVertices; ++i)
        {
            buffer[i].normal = desc.normals ? desc.normals[i] : Float3();
            buffer[i].tangent = desc.tangents ? desc.tangents[i] : Float3();
            buffer[i].texCoord = desc.texCoords ? desc.texCoords[i] : Float2();

            RT_ASSERT(buffer[i].normal.IsValid(), "Corrupted normal vector");
            RT_ASSERT(buffer[i].tangent.IsValid(), "Corrupted tangent vector");
            RT_ASSERT(buffer[i].texCoord.IsValid(), "Corrupted texture coordinates");
            RT_ASSERT(Abs(1.0f - buffer[i].normal.Length()) < 0.0001f, "Normal vector is not normalized");
            RT_ASSERT(Abs(1.0f - buffer[i].tangent.Length()) < 0.0001f, "Tangent vector is not normalized");
            RT_ASSERT(Abs(Float3::Dot(buffer[i].normal, buffer[i].tangent)) < 0.0001f, "Normal and tangent vectors are not orthogonal");
        }
    }

    if (desc.numMaterials > 0u)
    {
        Material** buffer = reinterpret_cast<Material**>(mBuffer + mMaterialBufferOffset);

        mMaterials.Resize(desc.numMaterials);
        for (Uint32 i = 0; i < desc.numMaterials; ++i)
        {
            buffer[i] = desc.materials[i].get();
            mMaterials[i] = desc.materials[i];
        }
    }

    mNumVertices = desc.numVertices;
    mNumTriangles = desc.numTriangles;

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
    RT_ASSERT(materialIndex < mMaterials.Size());

    const Material** materialBufferData = reinterpret_cast<const Material**>(mBuffer + mMaterialBufferOffset);
    return materialBufferData[materialIndex];
}

const math::ProcessedTriangle& VertexBuffer::GetTriangle(const Uint32 triangleIndex) const
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
