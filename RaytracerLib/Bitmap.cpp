#include "PCH.h"
#include "Bitmap.h"
#include "Logger.h"

namespace rt {


size_t Bitmap::BitsPerPixel(Format format)
{
    switch (format)
    {
    case Format::Unknown:            return 0;
    case Format::B8G8R8A8_Uint:      return 8 * 4;
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
        RT_LOG_ERROR("Bitmap width is too large");
        return false;
    }

    if (height >= UINT16_MAX)
    {
        RT_LOG_ERROR("Bitmap height is too large");
        return false;
    }

    const size_t dataSize = width * height * BitsPerPixel(format) / 8;
    if (dataSize == 0)
    {
        RT_LOG_ERROR("Invalid bitmap format");
        return false;
    }

    Release();

    // align to cache line
    mData = _aligned_malloc(dataSize, 64);
    if (!mData)
    {
        RT_LOG_ERROR("Memory allocation failed");
        return false;
    }

    if (data)
    {
        memcpy(mData, data, dataSize);
    }

    mWidth = (Uint16)width;
    mHeight = (Uint16)height;
    mFormat = format;
    return true;
}

void Bitmap::SetPixel(Uint32 x, Uint32 y, const math::Vector4& value)
{
    using namespace math;

    const Uint32 offset = mWidth * y + x;

    switch (mFormat)
    {
        case Format::B8G8R8A8_Uint:
        {
            Uint8* target = reinterpret_cast<Uint8*>(mData) + (4 * offset);
            const Vector4 clampedValue = value.Swizzle<2, 1, 0, 3>() * 255.0f;
            clampedValue.Store4(target);
            break;
        }

        case Format::R32G32B32A32_Float:
        {
            math::Vector4* target = reinterpret_cast<math::Vector4*>(mData) + offset;
            *target = value;
            break;
        }

        default:
            break;
    }
}

math::Vector4 Bitmap::GetPixel(Uint32 x, Uint32 y) const
{
    using namespace math;

    const Uint32 offset = mWidth * y + x;

    switch (mFormat)
    {
        case Format::B8G8R8A8_Uint:
        {
            const Uint8* source = reinterpret_cast<Uint8*>(mData) + (4 * offset);
            return math::Vector4::Load4(source).Swizzle<2, 1, 0, 3>();
            break;
        }

        case Format::R32G32B32A32_Float:
        {
            const math::Vector4* source = reinterpret_cast<math::Vector4*>(mData) + offset;
            return *source;
        }

        default:
            break;
    }

    return math::Vector4();
}

void Bitmap::Zero()
{
    if (mData)
    {
        size_t dataSize = mWidth * mHeight * BitsPerPixel(mFormat) / 8;
        ZeroMemory(mData, dataSize);
    }
}

} // namespace rt
