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
    if (width >= MaxSize)
    {
        RT_LOG_ERROR("Bitmap width is too large");
        return false;
    }

    if (height >= MaxSize)
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
    mData = _aligned_malloc(dataSize, RT_CACHE_LINE_SIZE);
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

void Bitmap::WriteHorizontalLine(Uint32 y, const math::Vector4* values)
{
    const Uint32 offset = mWidth * y;

    switch (mFormat)
    {
        case Format::B8G8R8A8_Uint:
        {
            Uint8* target = reinterpret_cast<Uint8*>(mData) + 4 * offset;
            for (Uint32 x = 0; x < mWidth; ++x)
            {
                const math::Vector4 clampedValue = values[x].Swizzle<2, 1, 0, 3>() * 255.0f;
                clampedValue.Store4(target + 4 * x);
            }
            break;
        }

        case Format::R32G32B32A32_Float:
        {
            math::Vector4* target = reinterpret_cast<math::Vector4*>(mData) + offset;
            for (Uint32 x = 0; x < mWidth; ++x)
            {
                target[x] = values[x];
            }
            break;
        }

        default:
            break;
    }
}

void Bitmap::WriteVerticalLine(Uint32 x, const math::Vector4* values)
{
    switch (mFormat)
    {
        case Format::B8G8R8A8_Uint:
        {
            Uint8* target = reinterpret_cast<Uint8*>(mData) + 4 * x;
            for (Uint32 y = 0; y < mHeight; ++y)
            {
                const math::Vector4 clampedValue = values[y].Swizzle<2, 1, 0, 3>() * 255.0f;
                clampedValue.Store4(target + 4 * mWidth * y);
            }
            break;
        }

        case Format::R32G32B32A32_Float:
        {
            math::Vector4* target = reinterpret_cast<math::Vector4*>(mData) + x;
            for (Uint32 y = 0; y < mHeight; ++y)
            {
                target[y * mWidth] = values[y];
            }
            break;
        }

        default:
            break;
    }
}

void Bitmap::ReadHorizontalLine(Uint32 y, math::Vector4* outValues) const
{
    const Uint32 offset = mWidth * y;

    switch (mFormat)
    {
        case Format::B8G8R8A8_Uint:
        {
            const Uint8* source = reinterpret_cast<Uint8*>(mData) + (4 * offset);
            for (Uint32 x = 0; x < mWidth; ++x)
            {
                outValues[x] = math::Vector4::Load4(source + 4 * x).Swizzle<2, 1, 0, 3>();
            }
            break;
        }

        case Format::R32G32B32A32_Float:
        {
            const math::Vector4* source = reinterpret_cast<math::Vector4*>(mData) + offset;
            for (Uint32 x = 0; x < mWidth; ++x)
            {
                outValues[x] = source[x];
            }
            break;
        }

        default:
            break;
    }
}

void Bitmap::ReadVerticalLine(Uint32 x, math::Vector4* outValues) const
{
    switch (mFormat)
    {
        case Format::B8G8R8A8_Uint:
        {
            const Uint8* source = reinterpret_cast<Uint8*>(mData) + 4 * x;
            for (Uint32 y = 0; y < mHeight; ++y)
            {
                outValues[x] = math::Vector4::Load4(source + 4 * mWidth * y).Swizzle<2, 1, 0, 3>();
            }
            break;
        }

        case Format::R32G32B32A32_Float:
        {
            const math::Vector4* source = reinterpret_cast<math::Vector4*>(mData) + x;
            for (Uint32 y = 0; y < mHeight; ++y)
            {
                outValues[y] = source[y * mWidth];
            }
            break;
        }

        default:
            break;
    }
}

void Bitmap::Zero()
{
    if (mData)
    {
        size_t dataSize = mWidth * mHeight * BitsPerPixel(mFormat) / 8;
        ZeroMemory(mData, dataSize);
    }
}

//////////////////////////////////////////////////////////////////////////

void Bitmap::BoxBlur_Internal(math::Vector4* targetLine, const math::Vector4* srcLine,
                              const Uint32 radius, const Uint32 width, const Float factor)
{
    Uint32 ti = 0;
    Uint32 li = 0;
    Uint32 ri = radius;

    math::Vector4 fv = srcLine[0];
    math::Vector4 lv = srcLine[width - 1];
    math::Vector4 val = fv * (Float)(radius + 1);

    for (Uint32 j = 0; j < radius; j++)
    {
        val += srcLine[ti + j];
    }

    for (Uint32 j = 0; j <= radius; j++)
    {
        val += srcLine[ri++] - fv;
        targetLine[ti++] = val * factor;
    }

    for (Uint32 j = radius + 1; j < width - radius; j++)
    {
        val += srcLine[ri++] - srcLine[li++];
        targetLine[ti++] = val * factor;
    }

    for (Uint32 j = width - radius; j < width; j++)
    {
        val += lv - srcLine[li++];
        targetLine[ti++] = val * factor;
    }
}

Bool Bitmap::VerticalBoxBlur(Bitmap& target, const Bitmap& src, const Uint32 radius)
{
    if (target.mWidth != src.mWidth || target.mHeight != src.mHeight)
    {
        RT_LOG_ERROR("Source and target image sizes do not match");
        return false;
    }

    math::Vector4 srcLine[MaxSize];
    math::Vector4 targetLine[MaxSize];

    const Float factor = 1.0f / (Float)(2 * radius + 1);

    for (Uint32 i = 0; i < src.mWidth; ++i)
    {
        src.ReadVerticalLine(i, srcLine);
        BoxBlur_Internal(targetLine, srcLine, radius, src.mHeight, factor);
        target.WriteVerticalLine(i, targetLine);
    }

    return true;
}

Bool Bitmap::HorizontalBoxBlur(Bitmap& target, const Bitmap& src, const Uint32 radius)
{
    if (target.mWidth != src.mWidth || target.mHeight != src.mHeight)
    {
        RT_LOG_ERROR("Source and target image sizes do not match");
        return false;
    }

    math::Vector4 srcLine[MaxSize];
    math::Vector4 targetLine[MaxSize];

    const Float factor = 1.0f / (Float)(2 * radius + 1);

    for (Uint32 i = 0; i < src.mHeight; ++i)
    {
        src.ReadHorizontalLine(i, srcLine);
        BoxBlur_Internal(targetLine, srcLine, radius, src.mWidth, factor);
        target.WriteHorizontalLine(i, targetLine);
    }

    return true;
}

Bool Bitmap::Blur(Bitmap& target, const Bitmap& src, const Float sigma, const Uint32 n)
{
    if (target.mWidth != src.mWidth || target.mHeight != src.mHeight)
    {
        RT_LOG_ERROR("Source and target image sizes do not match");
        return false;
    }

    // based on http://blog.ivank.net/fastest-gaussian-blur.html

    Float wIdeal = sqrtf((12.0f * sigma * sigma / n) + 1.0f);
    Uint32 wl = (Uint32)floorf(wIdeal);

    if (wl % 2 == 0)
        wl--;

    Uint32 wu = wl + 2;
    Float mIdeal = (12.0f * sigma * sigma - n * wl*wl - 4.0f * n*wl - 3.0f * n) / (-4.0f * wl - 4.0f);
    Float m = roundf(mIdeal);

    for (Uint32 i = 0; i < n; ++i)
    {
        const Uint32 radius = i < m ? wl : wu;
        VerticalBoxBlur(target, i == 0 ? src : target, radius);
        HorizontalBoxBlur(target, target, radius);
    }

    return true;
}

} // namespace rt
