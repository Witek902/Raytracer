#include "PCH.h"
#include "VertexBuffer.h"

#include "Logger.h"


namespace rt {

using namespace math;

template<typename T>
static T RoundUp(T x, T multiple)
{
    T remainder = x % multiple;
    if (remainder == 0)
        return x;

    return x + multiple - remainder;
}


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
    mNumTriangles = 0;
    mVertexIndexBufferOffset = 0;
    mPositionsBufferOffset = 0;
    mNormalsBufferOffset = 0;
    mTangentsBufferOffset = 0;
    mTexCoordsBufferOffset = 0;
    mMaterialIndexBufferOffset = 0;
    mMaterialBufferOffset = 0;
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

    if (GetElementSize(desc.positionsFormat) == 0)
    {
        RT_LOG_ERROR("Invalid positions buffer format");
        return false;
    }

    if (!desc.vertexIndexBuffer)
    {
        RT_LOG_ERROR("Index buffer must be provided");
        return false;
    }

    if (desc.vertexIndexFormat != VertexDataFormat::Int8 &&
        desc.vertexIndexFormat != VertexDataFormat::Int16 &&
        desc.vertexIndexFormat != VertexDataFormat::Int32)
    {
        RT_LOG_ERROR("Invalid index buffer format (only integer types are supported)");
        return false;
    }


    Uint32 bufferSizeRequired = 0;

    const Uint32 indexBufferSize = 3 * GetElementSize(desc.vertexIndexFormat) * desc.numTriangles;
    const Uint32 positionsBufferSize = 3 * GetElementSize(desc.positionsFormat) * desc.numVertices;
    Uint32 materialBufferSize = 0;
    Uint32 materialIndexBufferSize = 0;
    Uint32 normalsBufferSize = 0;
    Uint32 tangentsBufferSize = 0;
    Uint32 texCoordsBufferSize = 0;

    bufferSizeRequired = RoundUp<Uint32>(indexBufferSize, 4);
    mPositionsBufferOffset = bufferSizeRequired;
    bufferSizeRequired += RoundUp<Uint32>(positionsBufferSize, 4);

    mNormalsFormat = desc.normalsFormat;
    if (desc.normalsFormat != VertexDataFormat::None && desc.normals)
    {
        normalsBufferSize = 3 * desc.numVertices * GetElementSize(desc.normalsFormat);
        mNormalsBufferOffset = bufferSizeRequired;
        bufferSizeRequired += RoundUp<Uint32>(normalsBufferSize, 4);
    }

    mTangentsFormat = desc.tangentsFormat;
    if (desc.tangentsFormat != VertexDataFormat::None && desc.tangents)
    {
        tangentsBufferSize = 3 * desc.numVertices * GetElementSize(desc.tangentsFormat);
        mTangentsBufferOffset = bufferSizeRequired;
        bufferSizeRequired += RoundUp<Uint32>(tangentsBufferSize, 4);
    }

    mTexCoordsFormat = desc.texCoordsFormat;
    if (desc.texCoordsFormat != VertexDataFormat::None && desc.texCoords)
    {
        texCoordsBufferSize = 2 * desc.numVertices * GetElementSize(desc.texCoordsFormat);
        mTexCoordsBufferOffset = bufferSizeRequired;
        bufferSizeRequired += RoundUp<Uint32>(texCoordsBufferSize, 4);
    }

    if (desc.numMaterials > 0 && desc.materialIndexBuffer && desc.materials)
    {
        if (desc.materialIndexFormat != VertexDataFormat::Int8 &&
            desc.materialIndexFormat != VertexDataFormat::Int16 &&
            desc.materialIndexFormat != VertexDataFormat::Int32)
        {
            RT_LOG_ERROR("Invalid index buffer format (only integer types are supported)");
            return false;
        }

        materialBufferSize = desc.numMaterials * sizeof(Material*);
        materialIndexBufferSize = GetElementSize(desc.materialIndexFormat) * desc.numTriangles;

        mMaterialIndexBufferOffset = bufferSizeRequired;
        bufferSizeRequired += RoundUp<Uint32>(materialIndexBufferSize, 4);

        mMaterialBufferOffset = bufferSizeRequired;
        bufferSizeRequired += RoundUp<Uint32>(materialBufferSize, 8);
    }

    // margin
    bufferSizeRequired += 16;

    // create the buffer
    RT_LOG_DEBUG("Allocating vertex buffer for mesh, size = %u", bufferSizeRequired);
    mBuffer = (char*)_aligned_malloc(bufferSizeRequired, 64);
    if (!mBuffer)
    {
        RT_LOG_ERROR("Memory allocation failed");
        return false;
    }

    // copy source data
    {
        memcpy(mBuffer, desc.vertexIndexBuffer, indexBufferSize);
        memcpy(mBuffer + mPositionsBufferOffset, desc.positions, positionsBufferSize);

        if (normalsBufferSize > 0)
        {
            mNormalsFormat = desc.normalsFormat;
            memcpy(mBuffer + mNormalsBufferOffset, desc.normals, normalsBufferSize);
        }

        if (tangentsBufferSize > 0)
        {
            mTangentsFormat = desc.tangentsFormat;
            memcpy(mBuffer + mTangentsBufferOffset, desc.tangents, tangentsBufferSize);
        }

        if (texCoordsBufferSize > 0)
        {
            mTexCoordsFormat = desc.texCoordsFormat;
            memcpy(mBuffer + mTexCoordsBufferOffset, desc.texCoords, texCoordsBufferSize);
        }

        if (materialBufferSize > 0 && materialIndexBufferSize > 0)
        {
            mMaterialIndexFormat = desc.vertexIndexFormat;
            memcpy(mBuffer + mMaterialBufferOffset, desc.materials, materialBufferSize);
            memcpy(mBuffer + mMaterialIndexBufferOffset, desc.materialIndexBuffer, materialIndexBufferSize);
        }
    }

    mVertexIndexFormat = desc.vertexIndexFormat;
    mPositionsFormat = desc.positionsFormat;
    mNumVertices = desc.numVertices;
    mNumTriangles = desc.numTriangles;

    return true;
}

void VertexBuffer::GetVertexIndices(const Uint32 triangleIndex, VertexIndices& indices) const
{
    const char* data = mBuffer + mVertexIndexBufferOffset;
    const Uint32 indexOffset = 3 * triangleIndex;

    switch (mVertexIndexFormat)
    {
        case VertexDataFormat::Int8:
        {
            const unsigned char* typedDataBuffer = reinterpret_cast<const unsigned char*>(data);
            indices.i0 = typedDataBuffer[indexOffset];
            indices.i1 = typedDataBuffer[indexOffset + 1];
            indices.i2 = typedDataBuffer[indexOffset + 2];
            break;
        }

        case VertexDataFormat::Int16:
        {
            const unsigned short* typedDataBuffer = reinterpret_cast<const unsigned short*>(data);
            indices.i0 = typedDataBuffer[indexOffset];
            indices.i1 = typedDataBuffer[indexOffset + 1];
            indices.i2 = typedDataBuffer[indexOffset + 2];
            break;
        }

        case VertexDataFormat::Int32:
        {
            const unsigned int* typedDataBuffer = reinterpret_cast<const unsigned int*>(data);
            indices.i0 = typedDataBuffer[indexOffset];
            indices.i1 = typedDataBuffer[indexOffset + 1];
            indices.i2 = typedDataBuffer[indexOffset + 2];
            break;
        }
    }
}

const Material* VertexBuffer::GetMaterial(const Uint32 triangleIndex) const
{
    const char* data = mBuffer + mMaterialIndexBufferOffset;
    Uint32 materialIndex = 0;

    switch (mMaterialIndexFormat)
    {
        case VertexDataFormat::Int8:
        {
            const unsigned char* typedDataBuffer = reinterpret_cast<const unsigned char*>(data);
            materialIndex = typedDataBuffer[triangleIndex];
            break;
        }

        case VertexDataFormat::Int16:
        {
            const unsigned short* typedDataBuffer = reinterpret_cast<const unsigned short*>(data);
            materialIndex = typedDataBuffer[triangleIndex];
            break;
        }

        case VertexDataFormat::Int32:
        {
            const unsigned int* typedDataBuffer = reinterpret_cast<const unsigned int*>(data);
            materialIndex = typedDataBuffer[triangleIndex];
            break;
        }

        default:
            return nullptr;
    }

    const Material** materialBufferData = reinterpret_cast<const Material**>(mBuffer + mMaterialBufferOffset);
    return materialBufferData[materialIndex];
}

Uint32 VertexBuffer::GetElementSize(VertexDataFormat format)
{
    switch (format)
    {
    case VertexDataFormat::Float:
    case VertexDataFormat::Int32:
        return 4;
    case VertexDataFormat::Int16:
    case VertexDataFormat::HalfFloat:
        return 2;
    case VertexDataFormat::Int8:
        return 1;
    default:
        return 0;
    }
}

void VertexBuffer::ExtractTriangleData2(const void* dataBuffer, VertexDataFormat format, const VertexIndices& indices, math::Triangle& data)
{
    if (format == VertexDataFormat::Float)
    {
        const Float* typedDataBuffer = reinterpret_cast<const Float*>(dataBuffer);
        const Float* v0 = typedDataBuffer + 2 * indices.i0;
        const Float* v1 = typedDataBuffer + 2 * indices.i1;
        const Float* v2 = typedDataBuffer + 2 * indices.i2;

        // clear ZW components (there can be garbage)
        data.v0 = Vector4(v0) & VECTOR_MASK_XY;
        data.v1 = Vector4(v1) & VECTOR_MASK_XY;
        data.v2 = Vector4(v2) & VECTOR_MASK_XY;
    }
    else if (format == VertexDataFormat::Int16)
    {
        const Int16* typedDataBuffer = reinterpret_cast<const Int16*>(dataBuffer);
        const Int16* v0 = typedDataBuffer + 2 * indices.i0;
        const Int16* v1 = typedDataBuffer + 2 * indices.i1;
        const Int16* v2 = typedDataBuffer + 2 * indices.i2;

        // TODO write better Vector4::Load2 method
        data.v0 = Vector4((Float)v0[0], (Float)v0[1]);
        data.v1 = Vector4((Float)v1[0], (Float)v1[1]);
        data.v2 = Vector4((Float)v2[0], (Float)v2[1]);
    }
    else if (format == VertexDataFormat::Int8)
    {
        const Int8* typedDataBuffer = reinterpret_cast<const Int8*>(dataBuffer);
        const Int8* v0 = typedDataBuffer + 2 * indices.i0;
        const Int8* v1 = typedDataBuffer + 2 * indices.i1;
        const Int8* v2 = typedDataBuffer + 2 * indices.i2;

        // TODO write better Vector4::Load2 method
        data.v0 = Vector4((Float)v0[0], (Float)v0[1]);
        data.v1 = Vector4((Float)v1[0], (Float)v1[1]);
        data.v2 = Vector4((Float)v2[0], (Float)v2[1]);
    }
    else
    {
        // TODO
    }
}

void VertexBuffer::ExtractTriangleData3(const void* dataBuffer, VertexDataFormat format, const VertexIndices& indices, math::Triangle& data)
{
    if (format == VertexDataFormat::Float)
    {
        const Float* typedDataBuffer = reinterpret_cast<const Float*>(dataBuffer);
        const Float* v0 = typedDataBuffer + 3 * indices.i0;
        const Float* v1 = typedDataBuffer + 3 * indices.i1;
        const Float* v2 = typedDataBuffer + 3 * indices.i2;

        // clear W component (there can be garbage)
        data.v0 = Vector4(v0) & VECTOR_MASK_XYZ;
        data.v1 = Vector4(v1) & VECTOR_MASK_XYZ;
        data.v2 = Vector4(v2) & VECTOR_MASK_XYZ;
    }
    else if (format == VertexDataFormat::Int16)
    {
        const Int16* typedDataBuffer = reinterpret_cast<const Int16*>(dataBuffer);
        const Int16* v0 = typedDataBuffer + 3 * indices.i0;
        const Int16* v1 = typedDataBuffer + 3 * indices.i1;
        const Int16* v2 = typedDataBuffer + 3 * indices.i2;

        // TODO write better Vector4::Load3 method
        data.v0 = Vector4((Float)v0[0], (Float)v0[1], (Float)v0[2]);
        data.v1 = Vector4((Float)v1[0], (Float)v1[1], (Float)v1[2]);
        data.v2 = Vector4((Float)v2[0], (Float)v2[1], (Float)v2[2]);
    }
    else if (format == VertexDataFormat::Int8)
    {
        const Int8* typedDataBuffer = reinterpret_cast<const Int8*>(dataBuffer);
        const Int8* v0 = typedDataBuffer + 3 * indices.i0;
        const Int8* v1 = typedDataBuffer + 3 * indices.i1;
        const Int8* v2 = typedDataBuffer + 3 * indices.i2;

        // TODO write better Vector4::Load3 method
        data.v0 = Vector4((Float)v0[0], (Float)v0[1], (Float)v0[2]);
        data.v1 = Vector4((Float)v1[0], (Float)v1[1], (Float)v1[2]);
        data.v2 = Vector4((Float)v2[0], (Float)v2[1], (Float)v2[2]);
    }
}

void VertexBuffer::GetVertexPositions(const VertexIndices& indices, math::Triangle& data) const
{
    ExtractTriangleData3(mBuffer + mPositionsBufferOffset, mPositionsFormat, indices, data);
}

void VertexBuffer::GetVertexNormals(const VertexIndices& indices, math::Triangle& data) const
{
    if (mNormalsFormat == VertexDataFormat::None)
    {
        data = math::Triangle();
        return;
    }

    ExtractTriangleData3(mBuffer + mNormalsBufferOffset, mNormalsFormat, indices, data);
}

void VertexBuffer::GetVertexTangents(const VertexIndices& indices, math::Triangle& data) const
{
    if (mTangentsFormat == VertexDataFormat::None)
    {
        data = math::Triangle();
        return;
    }

    ExtractTriangleData3(mBuffer + mTangentsBufferOffset, mTangentsFormat, indices, data);
}

void VertexBuffer::GetVertexTexCoords(const VertexIndices& indices, math::Triangle& data) const
{
    if (mTexCoordsFormat == VertexDataFormat::None)
    {
        data = math::Triangle();
        return;
    }

    ExtractTriangleData3(mBuffer + mTexCoordsBufferOffset, mTexCoordsFormat, indices, data);
}


} // namespace rt
