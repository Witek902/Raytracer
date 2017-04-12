#include "PCH.h"
#include "Bitmap.h"
#include "Logger.h"

namespace rt {


size_t Bitmap::BitsPerPixel(Format format)
{
    switch (format)
    {
    case Format::Unknown:            return 0;
    case Format::R8G8B8A8_Uint:      return 8 * 4;
    case Format::R32G32B32A32_Float: return 8 * 4 * 4;
    }

    return 0;
}

Bitmap::Bitmap()
    : mData(nullptr)
    , mWidth(0)
    , mHeight(0)
    , mFormat(Format::Unknown)
{ }

Bitmap::~Bitmap()
{
    Release();
}

void Bitmap::Release()
{
    if (mData)
    {
        _aligned_free(mData);
        mData = nullptr;
    }

    mWidth = 0;
    mHeight = 0;
    mFormat = Format::Unknown;
}

Bool Bitmap::Init(Uint32 width, Uint32 height, Format format, const void* data)
{
    if (width >= UINT16_MAX)
    {
        LOG_ERROR("Bitmap width is too large");
        return false;
    }

    if (height >= UINT16_MAX)
    {
        LOG_ERROR("Bitmap height is too large");
        return false;
    }

    const size_t dataSize = width * height * BitsPerPixel(format) / 8;
    if (dataSize == 0)
    {
        LOG_ERROR("Invalid bitmap format");
        return false;
    }

    Release();

    // align to cache line
    mData = _aligned_malloc(dataSize, 64);
    if (!mData)
    {
        LOG_ERROR("Memory allocation failed");
        return false;
    }

    if (data)
    {
        memcpy(mData, data, dataSize);
    }

    mWidth = (Uint16)width;
    mHeight = (Uint16)height;
    mFormat = format;

    return false;
}

void Bitmap::SetPixel(Uint32 x, Uint32 y, const math::Vector& value)
{
    using namespace math;

    const Uint32 offset = mWidth * y + x;

    switch (mFormat)
    {
        case Format::R8G8B8A8_Uint:
        {
            Uint8* target = reinterpret_cast<Uint8*>(mData) + (4 * offset);
            const Vector clampedValue = value.Clamped(Vector(), VECTOR_ONE) * 255.0f;
            clampedValue.Store4(target);
            break;
        }

        case Format::R32G32B32A32_Float:
        {
            math::Vector* target = reinterpret_cast<math::Vector*>(mData) + offset;
            *target = value;
            break;
        }
    }
}

math::Vector Bitmap::GetPixel(Uint32 x, Uint32 y) const
{
    using namespace math;

    const Uint32 offset = mWidth * y + x;

    switch (mFormat)
    {
        case Format::R8G8B8A8_Uint:
        {
            const Uint8* source = reinterpret_cast<Uint8*>(mData) + (4 * offset);
            return math::Vector::Load4(source);
            break;
        }

        case Format::R32G32B32A32_Float:
        {
            const math::Vector* source = reinterpret_cast<math::Vector*>(mData) + offset;
            return *source;
        }
    }

    return math::Vector();
}

} // namespace rt
