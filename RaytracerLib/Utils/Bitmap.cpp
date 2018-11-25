#include "PCH.h"
#include "Bitmap.h"
#include "Logger.h"
#include "BlockCompression.h"
#include "Timer.h"
#include "Math/Half.h"

#include <string.h>

namespace rt {

using namespace math;

Uint32 Bitmap::BitsPerPixel(Format format)
{
    switch (format)
    {
    case Format::Unknown:               return 0;
    case Format::R8_Uint:               return 8;
    case Format::B8G8R8_Uint:           return 8 * 3;
    case Format::B8G8R8A8_Uint:         return 8 * 4;
    case Format::R32G32B32_Float:       return 8 * sizeof(Float) * 3;
    case Format::R32G32B32A32_Float:    return 8 * sizeof(Float) * 4;
    case Format::R16G16B16_Half:        return 8 * sizeof(Uint16) * 3;
    case Format::BC1:                   return 4;
    case Format::BC4:                   return 4;
    case Format::BC5:                   return 8;
    }

    return 0;
}

const char* Bitmap::FormatToString(Format format)
{
    switch (format)
    {
    case Format::R8_Uint:               return "R8_Uint";
    case Format::B8G8R8_Uint:           return "B8G8R8_Uint";
    case Format::B8G8R8A8_Uint:         return "B8G8R8A8_Uint";
    case Format::R32G32B32_Float:       return "R32G32B32_Float";
    case Format::R32G32B32A32_Float:    return "R32G32B32A32_Float";
    case Format::R16G16B16_Half:        return "R16G16B16_Half";
    case Format::BC1:                   return "BC1";
    case Format::BC4:                   return "BC4";
    case Format::BC5:                   return "BC5";
    }

    return "<unknown>";
}

size_t Bitmap::GetDataSize(Uint32 width, Uint32 height, Format format)
{
    const Uint64 dataSize = (Uint64)width * (Uint64)height * (Uint64)BitsPerPixel(format) / 8;

    if (dataSize >= (Uint64)std::numeric_limits<size_t>::max())
    {
        return std::numeric_limits<size_t>::max();
    }

    return (size_t)dataSize;
}

Bitmap::Bitmap(const char* debugName)
    : mData(nullptr)
    , mWidth(0)
    , mHeight(0)
    , mFormat(Format::Unknown)
    , mLinearSpace(false)
    , mDebugName(debugName)
{ }

Bitmap::~Bitmap()
{
    Release();

    RT_LOG_INFO("Releasing bitmap '%s'", mDebugName.c_str());
}

void Bitmap::Clear()
{
    if (mData)
    {
        memset(mData, 0, GetDataSize(mWidth, mHeight, mFormat));
    }
}

void Bitmap::Release()
{
    if (mData)
    {
        AlignedFree(mData);
        mData = nullptr;
    }

    mWidth = 0;
    mHeight = 0;
    mFormat = Format::Unknown;
}

Bool Bitmap::Init(Uint32 width, Uint32 height, Format format, const void* data, bool linearSpace)
{
    const size_t dataSize = GetDataSize(width, height, format);
    if (dataSize == 0)
    {
        RT_LOG_ERROR("Invalid bitmap format");
        return false;
    }
    if (dataSize == std::numeric_limits<size_t>::max())
    {
        RT_LOG_ERROR("Texture is too big");
        return false;
    }

    Release();

    // align to cache line
    const Uint32 marigin = RT_CACHE_LINE_SIZE;
    mData = (Uint8*)AlignedMalloc(dataSize + marigin, RT_CACHE_LINE_SIZE);
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
    mSize = Vector4((Float)width, (Float)height, 0.0f, 0.0f);
    mFormat = format;
    mLinearSpace = linearSpace;

    return true;
}

Bool Bitmap::Copy(Bitmap& target, const Bitmap& source)
{
    if (target.mWidth != source.mWidth || target.mHeight != source.mHeight)
    {
        RT_LOG_ERROR("Bitmap copy failed: bitmaps have different dimensions");
        return false;
    }

    if (target.mFormat != source.mFormat)
    {
        RT_LOG_ERROR("Bitmap copy failed: bitmaps have different formats");
        return false;
    }

    memcpy(target.GetData(), source.GetData(), GetDataSize(target.mWidth, target.mHeight, target.mFormat));
    return true;
}

Bool Bitmap::Load(const char* path)
{
    Timer timer;

    FILE* file = fopen(path, "rb");
    if (!file)
    {
        RT_LOG_ERROR("Failed to load source image from file '%hs'", path);
        return false;
    }

    if (!LoadBMP(file, path))
    {
        fseek(file, SEEK_SET, 0);

        if (!LoadDDS(file, path))
        {
            fseek(file, SEEK_SET, 0);

            if (!LoadEXR(file, path))
            {
                RT_LOG_ERROR("Failed to load '%hs' - unknown format", path);
                fclose(file);
                return false;
            }
        }
    }

    fclose(file);

    const float elapsedTime = static_cast<float>(1000.0 * timer.Stop());
    RT_LOG_INFO("Bitmap '%hs' loaded in %.3fms: format=%s, width=%u, height=%u", path, elapsedTime, FormatToString(mFormat), mWidth, mHeight);
    return true;
}

Vector4 Bitmap::GetPixel(Uint32 x, Uint32 y, const bool forceLinearSpace) const
{
    RT_ASSERT(x < mWidth);
    RT_ASSERT(y < mHeight);

    const Uint32 offset = mWidth * y + x;

    Vector4 color;
    switch (mFormat)
    {
        case Format::R8_Uint:
        {
            const Uint32 value = mData[offset];
            color = Vector4::FromInteger(value) * (1.0f / 255.0f);
            break;
        }

        case Format::B8G8R8_Uint:
        {
            const Uint8* source = mData + (3 * offset);
            color = Vector4::LoadBGR_UNorm(source);
            break;
        }

        case Format::B8G8R8A8_Uint:
        {
            const Uint8* source = mData + (4 * offset);
            color = Vector4::Load4(source).Swizzle<2, 1, 0, 3>() * (1.0f / 255.0f);
            break;
        }

        case Format::R32G32B32_Float:
        {
            const float* source = reinterpret_cast<const float*>(mData) + 3 * offset;
            color = Vector4(source) & VECTOR_MASK_XYZ;
            break;
        }

        case Format::R32G32B32A32_Float:
        {
            const Vector4* source = reinterpret_cast<const Vector4*>(mData) + offset;
            RT_PREFETCH_L2(source - mWidth);
            RT_PREFETCH_L2(source + mWidth);
            color = *source;
            break;
        }

        case Format::R16G16B16_Half:
        {
            const Half* source = reinterpret_cast<const Half*>(mData) + 3 * offset;
            color = Vector4::FromHalves(source) & VECTOR_MASK_XYZ;
            break;
        }

        case Format::BC1:
        {
            const Uint32 flippedY = mHeight - 1 - y;
            color = DecodeBC1(reinterpret_cast<const Uint8*>(mData), x, flippedY, mWidth);
            break;
        }

        case Format::BC4:
        {
            const Uint32 flippedY = mHeight - 1 - y;
            color = DecodeBC4(reinterpret_cast<const Uint8*>(mData), x, flippedY, mWidth);
            break;
        }

        case Format::BC5:
        {
            const Uint32 flippedY = mHeight - 1 - y;
            color = DecodeBC5(reinterpret_cast<const Uint8*>(mData), x, flippedY, mWidth);
            break;
        }

        default:
        {
            RT_FATAL("Unsupported bitmap format");
            color = Vector4::Zero();
        }
    }

    if (!mLinearSpace && !forceLinearSpace)
    {
        color *= color;
    }

    return color;
}

void Bitmap::GetPixelBlock(Uint32 x0, Uint32 y0, Uint32 x1, Uint32 y1, const bool forceLinearSpace,
    math::Vector4& outColor0, math::Vector4& outColor1, math::Vector4& outColor2, math::Vector4& outColor3) const
{
    RT_ASSERT(x0 < mWidth);
    RT_ASSERT(y0 < mHeight);
    RT_ASSERT(x1 < mWidth);
    RT_ASSERT(y1 < mHeight);

    const Uint32 offset0 = mWidth * y0 + x0;
    const Uint32 offset1 = mWidth * y0 + x1;
    const Uint32 offset2 = mWidth * y1 + x0;
    const Uint32 offset3 = mWidth * y1 + x1;

    constexpr float byteScale = 1.0f / 255.0f;

    switch (mFormat)
    {
        case Format::R8_Uint:
        {
            const Uint32 value0 = mData[offset0];
            const Uint32 value1 = mData[offset1];
            const Uint32 value2 = mData[offset2];
            const Uint32 value3 = mData[offset3];
            outColor0 = Vector4::FromInteger(value0) * byteScale;
            outColor1 = Vector4::FromInteger(value1) * byteScale;
            outColor2 = Vector4::FromInteger(value2) * byteScale;
            outColor3 = Vector4::FromInteger(value3) * byteScale;
            break;
        }

        case Format::B8G8R8_Uint:
        {
            outColor0 = Vector4::LoadBGR_UNorm(mData + 3u * offset0);
            outColor1 = Vector4::LoadBGR_UNorm(mData + 3u * offset1);
            outColor2 = Vector4::LoadBGR_UNorm(mData + 3u * offset2);
            outColor3 = Vector4::LoadBGR_UNorm(mData + 3u * offset3);
            break;
        }

        case Format::B8G8R8A8_Uint:
        {
            outColor0 = Vector4::Load4(mData + 4 * offset0).Swizzle<2, 1, 0, 3>() * byteScale;
            outColor1 = Vector4::Load4(mData + 4 * offset1).Swizzle<2, 1, 0, 3>() * byteScale;
            outColor2 = Vector4::Load4(mData + 4 * offset2).Swizzle<2, 1, 0, 3>() * byteScale;
            outColor3 = Vector4::Load4(mData + 4 * offset3).Swizzle<2, 1, 0, 3>() * byteScale;
            break;
        }

        case Format::R32G32B32_Float:
        {
            const float* source0 = reinterpret_cast<const float*>(mData) + 3u * offset0;
            const float* source1 = reinterpret_cast<const float*>(mData) + 3u * offset1;
            const float* source2 = reinterpret_cast<const float*>(mData) + 3u * offset2;
            const float* source3 = reinterpret_cast<const float*>(mData) + 3u * offset3;
            outColor0 = Vector4(source0) & VECTOR_MASK_XYZ;
            outColor1 = Vector4(source1) & VECTOR_MASK_XYZ;
            outColor2 = Vector4(source2) & VECTOR_MASK_XYZ;
            outColor3 = Vector4(source3) & VECTOR_MASK_XYZ;
            break;
        }

        case Format::R32G32B32A32_Float:
        {
            outColor0 = reinterpret_cast<const Vector4*>(mData)[offset0];
            outColor1 = reinterpret_cast<const Vector4*>(mData)[offset1];
            outColor2 = reinterpret_cast<const Vector4*>(mData)[offset2];
            outColor3 = reinterpret_cast<const Vector4*>(mData)[offset3];
            break;
        }

        case Format::R16G16B16_Half:
        {
            const Half* source0 = reinterpret_cast<const Half*>(mData) + 3 * offset0;
            const Half* source1 = reinterpret_cast<const Half*>(mData) + 3 * offset1;
            const Half* source2 = reinterpret_cast<const Half*>(mData) + 3 * offset2;
            const Half* source3 = reinterpret_cast<const Half*>(mData) + 3 * offset3;
            outColor0 = Vector4::FromHalves(source0) & VECTOR_MASK_XYZ;
            outColor1 = Vector4::FromHalves(source1) & VECTOR_MASK_XYZ;
            outColor2 = Vector4::FromHalves(source2) & VECTOR_MASK_XYZ;
            outColor3 = Vector4::FromHalves(source3) & VECTOR_MASK_XYZ;
            break;
        }

        /*
        case Format::BC1:
        {
            const Uint32 flippedY = mHeight - 1 - y;
            color = DecodeBC1(reinterpret_cast<const Uint8*>(mData), x, flippedY, mWidth);
            break;
        }

        case Format::BC4:
        {
            const Uint32 flippedY = mHeight - 1 - y;
            color = DecodeBC4(reinterpret_cast<const Uint8*>(mData), x, flippedY, mWidth);
            break;
        }

        case Format::BC5:
        {
            const Uint32 flippedY = mHeight - 1 - y;
            color = DecodeBC5(reinterpret_cast<const Uint8*>(mData), x, flippedY, mWidth);
            break;
        }
        */

        default:
        {
            RT_FATAL("Unsupported bitmap format");
            outColor0 = Vector4::Zero();
            outColor1 = Vector4::Zero();
            outColor2 = Vector4::Zero();
            outColor3 = Vector4::Zero();
        }
    }

    if (!mLinearSpace && !forceLinearSpace)
    {
        outColor0 *= outColor0;
        outColor1 *= outColor1;
        outColor2 *= outColor2;
        outColor3 *= outColor3;
    }
}

Vector4 Bitmap::Sample(Vector4 coords, const SamplerDesc& sampler) const
{
    RT_ASSERT(coords.IsValid());

    // perform wrapping

    __m128i intCoords = _mm_cvtps_epi32(_mm_floor_ps(coords));
    coords -= _mm_cvtepi32_ps(intCoords);

    coords *= mSize;
    intCoords = _mm_cvtps_epi32(_mm_floor_ps(coords));

    Int32 u0 = _mm_extract_epi32(intCoords, 0);
    Int32 v0 = _mm_extract_epi32(intCoords, 1);
    if (u0 >= mWidth) u0 = 0;
    if (v0 >= mHeight) v0 = 0;
    if (u0 < 0) u0 = 0;
    if (v0 < 0) v0 = 0;

    if (sampler.filter == TextureFilterMode::NearestNeighbor)
    {
        return GetPixel(u0, v0, sampler.forceLinearSpace);
    }
    else if (sampler.filter == TextureFilterMode::Bilinear)
    {
        Int32 u1 = u0 + 1;
        Int32 v1 = v0 + 1;
        if (u1 >= mWidth) u1 = 0;
        if (v1 >= mHeight) v1 = 0;

        Vector4 value00, value01, value10, value11;
        GetPixelBlock(u0, v0, u1, v1, sampler.forceLinearSpace, value00, value10, value01, value11);

        // bilinear interpolation
        const Float weightU = coords.x - (Float)u0;
        const Float weightV = coords.y - (Float)v0;
        const Vector4 value0 = Vector4::Lerp(value00, value01, weightV);
        const Vector4 value1 = Vector4::Lerp(value10, value11, weightV);
        const Vector4 result = Vector4::Lerp(value0, value1, weightU);

        RT_ASSERT(result.IsValid());
        return result;
    }
    
    RT_FATAL("Invalid filter mode");
    return Vector4::Zero();
}

} // namespace rt
