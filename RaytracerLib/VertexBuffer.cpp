#include "PCH.h"
#include "VertexBuffer.h"

namespace rt {


VertexBuffer::VertexBuffer()
    : mVertexIndexBuffer(nullptr)
    , mPositions(nullptr)
    , mNormals(nullptr)
    , mTangents(nullptr)
    , mTexCoords(nullptr)
    , mMaterialIndexBuffer(nullptr)
    , mVertexIndexFormat(VertexDataFormat::Int32)
    , mPositionsFormat(VertexDataFormat::Float)
    , mNormalsFormat(VertexDataFormat::Float)
    , mTangentsFormat(VertexDataFormat::Float)
    , mTexCoordsFormat(VertexDataFormat::Float)
    , mMaterialIndexFormat(VertexDataFormat::Int8)
{ }


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
    if (!mPositions)
    {
        data = TriangleData();
        return;
    }

    ExtractTriangleData3(mPositions, mPositionsFormat, indices, data);
}

void VertexBuffer::GetVertexNormals(const VertexIndices& indices, TriangleData& data) const
{
    if (!mNormals)
    {
        data = TriangleData();
        return;
    }

    ExtractTriangleData3(mNormals, mNormalsFormat, indices, data);
}

void VertexBuffer::GetVertexTangents(const VertexIndices& indices, TriangleData& data) const
{
    if (!mTangents)
    {
        data = TriangleData();
        return;
    }

    ExtractTriangleData3(mTangents, mTangentsFormat, indices, data);
}

void VertexBuffer::GetVertexTexCoords(const VertexIndices& indices, TriangleData& data) const
{
    if (!mTexCoords)
    {
        data = TriangleData();
        return;
    }

    ExtractTriangleData3(mTexCoords, mTexCoordsFormat, indices, data);
}


} // namespace rt
