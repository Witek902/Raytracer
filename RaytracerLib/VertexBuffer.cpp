#include "PCH.h"
#include "VertexBuffer.h"

#include "Logger.h"


namespace rt {


VertexBuffer::VertexBuffer()
    : mBuffer(nullptr)
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
        _aligned_free(mBuffer);
        mBuffer = nullptr;
    }

    mNumVertices = 0;
    mNumIndices = 0;
    mVertexIndexBufferOffset = 0;
    mPositionsBufferOffset = 0;
    mNormalsBufferOffset = 0;
    mTangentsBufferOffset = 0;
    mTexCoordsBufferOffset = 0;
    mMaterialIndexBufferOffset = 0;
    mVertexIndexFormat = VertexDataFormat::None;
    mPositionsFormat = VertexDataFormat::None;
    mNormalsFormat = VertexDataFormat::None;
    mTangentsFormat = VertexDataFormat::None;
    mTexCoordsFormat = VertexDataFormat::None;
    mMaterialIndexFormat = VertexDataFormat::None;
}

bool VertexBuffer::Initialize(const VertexBufferDesc& desc)
{
    Clear();

    if (desc.numIndices == 0)
    {
        LOG_WARNING("Creating empty vertex buffer");
        return true;
    }

    if (desc.numVertices > desc.numIndices)
    {
        LOG_WARNING("There are redundant vertices");
    }

    if (!desc.positions)
    {
        LOG_ERROR("Positions buffer must be provided");
        return false;
    }

    if (GetElementSize(desc.positionsFormat) == 0)
    {
        LOG_ERROR("Invalid positions buffer format");
        return false;
    }

    if (!desc.vertexIndexBuffer)
    {
        LOG_ERROR("Index buffer must be provided");
        return false;
    }

    if (GetElementSize(desc.vertexIndexFormat) == 0)
    {
        LOG_ERROR("Invalid index buffer format");
        return false;
    }

    // TODO overflow check

    const Uint32 indexBufferSize = GetElementSize(desc.vertexIndexFormat) * desc.numIndices;
    const Uint32 positionsBufferSize = 3 * GetElementSize(desc.positionsFormat) * desc.numVertices;

    Uint32 bufferSizeRequired = indexBufferSize + positionsBufferSize;
    mPositionsBufferOffset = indexBufferSize;

    mNormalsFormat = desc.normalsFormat;
    if (desc.normalsFormat != VertexDataFormat::None && desc.normals)
    {
        Uint32 size = GetElementSize(desc.normalsFormat);
        mNormalsBufferOffset = bufferSizeRequired;
        bufferSizeRequired += 3 * size * desc.numVertices; // TODO alignment
    }

    mTangentsFormat = desc.tangentsFormat;
    if (desc.tangentsFormat != VertexDataFormat::None && desc.tangents)
    {
        Uint32 size = GetElementSize(desc.tangentsFormat);
        mTangentsBufferOffset = bufferSizeRequired;
        bufferSizeRequired += 3 * size * desc.numVertices; // TODO alignment
    }

    mTexCoordsFormat = desc.texCoordsFormat;
    if (desc.texCoordsFormat != VertexDataFormat::None && desc.texCoords)
    {
        Uint32 size = GetElementSize(desc.texCoordsFormat);
        mTexCoordsBufferOffset = bufferSizeRequired;
        bufferSizeRequired += 2 * size * desc.numVertices; // TODO alignment
    }

    LOG_DEBUG("Allocating vertex buffer for mesh, size = %u", bufferSizeRequired);
    mBuffer = (char*)_aligned_malloc(bufferSizeRequired, 64);
    if (!mBuffer)
    {
        LOG_ERROR("Memory allocation failed");
        return false;
    }

    memcpy(mBuffer, desc.vertexIndexBuffer, indexBufferSize);
    memcpy(mBuffer + mPositionsBufferOffset, desc.positions, positionsBufferSize);

    return true;
}

void VertexBuffer::GetVertexIndices(const Uint32 triangleIndex, VertexIndices& indices) const
{
    RT_UNUSED(triangleIndex);
    RT_UNUSED(indices);

    // TODO
}

Uint32 VertexBuffer::GetMaterialIndex(const Uint32 triangleIndex) const
{
    RT_UNUSED(triangleIndex);

    // TODO

    return 0;
}

Uint32 VertexBuffer::GetElementSize(VertexDataFormat format)
{
    switch (format)
    {
    case VertexDataFormat::Float:
    case VertexDataFormat::Int32:
        return 4;
    case VertexDataFormat::Int16:
        return 2;
    case VertexDataFormat::Int8:
        return 1;
    }

    return 0;
}

void VertexBuffer::ExtractTriangleData2(const void* dataBuffer, VertexDataFormat format, const VertexIndices& indices, TriangleData& data)
{
    if (format == VertexDataFormat::Float)
    {
        const Float* typedDataBuffer = reinterpret_cast<const Float*>(dataBuffer);

        const Float* v0 = typedDataBuffer + indices.i0;
        const Float* v1 = typedDataBuffer + indices.i1;
        const Float* v2 = typedDataBuffer + indices.i2;

        // TODO write better Vector::Load2 method
        data.d0 = math::Vector(v0[0], v0[1]);
        data.d1 = math::Vector(v1[0], v1[1]);
        data.d2 = math::Vector(v2[0], v2[1]);
    }
    else if (format == VertexDataFormat::Int16)
    {
        const Int16* typedDataBuffer = reinterpret_cast<const Int16*>(dataBuffer);

        const Int16* v0 = typedDataBuffer + indices.i0;
        const Int16* v1 = typedDataBuffer + indices.i1;
        const Int16* v2 = typedDataBuffer + indices.i2;

        // TODO write better Vector::Load2 method
        data.d0 = math::Vector((Float)v0[0], (Float)v0[1]);
        data.d1 = math::Vector((Float)v1[0], (Float)v1[1]);
        data.d2 = math::Vector((Float)v2[0], (Float)v2[1]);
    }
    else if (format == VertexDataFormat::Int8)
    {
        const Int8* typedDataBuffer = reinterpret_cast<const Int8*>(dataBuffer);

        const Int8* v0 = typedDataBuffer + indices.i0;
        const Int8* v1 = typedDataBuffer + indices.i1;
        const Int8* v2 = typedDataBuffer + indices.i2;

        // TODO write better Vector::Load2 method
        data.d0 = math::Vector((Float)v0[0], (Float)v0[1]);
        data.d1 = math::Vector((Float)v1[0], (Float)v1[1]);
        data.d2 = math::Vector((Float)v2[0], (Float)v2[1]);
    }
    else
    {
        // TODO
    }
}

void VertexBuffer::ExtractTriangleData3(const void* dataBuffer, VertexDataFormat format, const VertexIndices& indices, TriangleData& data)
{
    if (format == VertexDataFormat::Float)
    {
        const Float* typedDataBuffer = reinterpret_cast<const Float*>(dataBuffer);

        const Float* v0 = typedDataBuffer + indices.i0;
        const Float* v1 = typedDataBuffer + indices.i1;
        const Float* v2 = typedDataBuffer + indices.i2;

        // TODO write better Vector::Load3 method
        data.d0 = math::Vector(v0[0], v0[1], v0[2]);
        data.d1 = math::Vector(v1[0], v1[1], v1[2]);
        data.d2 = math::Vector(v2[0], v2[1], v2[2]);
    }
    else if (format == VertexDataFormat::Int16)
    {
        const Int16* typedDataBuffer = reinterpret_cast<const Int16*>(dataBuffer);

        const Int16* v0 = typedDataBuffer + indices.i0;
        const Int16* v1 = typedDataBuffer + indices.i1;
        const Int16* v2 = typedDataBuffer + indices.i2;

        // TODO write better Vector::Load3 method
        data.d0 = math::Vector((Float)v0[0], (Float)v0[1], (Float)v0[2]);
        data.d1 = math::Vector((Float)v1[0], (Float)v1[1], (Float)v1[2]);
        data.d2 = math::Vector((Float)v2[0], (Float)v2[1], (Float)v2[2]);
    }
    else if (format == VertexDataFormat::Int8)
    {
        const Int8* typedDataBuffer = reinterpret_cast<const Int8*>(dataBuffer);

        const Int8* v0 = typedDataBuffer + indices.i0;
        const Int8* v1 = typedDataBuffer + indices.i1;
        const Int8* v2 = typedDataBuffer + indices.i2;

        // TODO write better Vector::Load3 method
        data.d0 = math::Vector((Float)v0[0], (Float)v0[1], (Float)v0[2]);
        data.d1 = math::Vector((Float)v1[0], (Float)v1[1], (Float)v1[2]);
        data.d2 = math::Vector((Float)v2[0], (Float)v2[1], (Float)v2[2]);
    }
    else
    {
        // TODO
    }
}

void VertexBuffer::GetVertexPositions(const VertexIndices& indices, TriangleData& data) const
{
    if (mPositionsFormat == VertexDataFormat::None)
    {
        data = TriangleData();
        return;
    }

    ExtractTriangleData3(mBuffer + mPositionsBufferOffset, mPositionsFormat, indices, data);
}

void VertexBuffer::GetVertexNormals(const VertexIndices& indices, TriangleData& data) const
{
    if (mNormalsFormat == VertexDataFormat::None)
    {
        data = TriangleData();
        return;
    }

    ExtractTriangleData3(mBuffer + mNormalsBufferOffset, mNormalsFormat, indices, data);
}

void VertexBuffer::GetVertexTangents(const VertexIndices& indices, TriangleData& data) const
{
    if (mTangentsFormat == VertexDataFormat::None)
    {
        data = TriangleData();
        return;
    }

    ExtractTriangleData3(mBuffer + mTangentsBufferOffset, mTangentsFormat, indices, data);
}

void VertexBuffer::GetVertexTexCoords(const VertexIndices& indices, TriangleData& data) const
{
    if (mTexCoordsFormat == VertexDataFormat::None)
    {
        data = TriangleData();
        return;
    }

    ExtractTriangleData3(mBuffer + mTexCoordsBufferOffset, mTexCoordsFormat, indices, data);
}


} // namespace rt
