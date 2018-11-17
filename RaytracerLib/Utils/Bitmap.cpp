#include "PCH.h"
#include "Bitmap.h"
#include "Logger.h"
#include "BlockCompression.h"
#include "Timer.h"
#include "Math/Half.h"

#include <string.h>

namespace rt {

using namespace math;

static_assert(sizeof(Bitmap) <= 64, "Bitmap structure is too big");

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

Bitmap::Bitmap()
    : mData(nullptr)
    , mWidth(0)
    , mHeight(0)
    , mFormat(Format::Unknown)
    , mLinearSpace(false)
{ }

Bitmap::~Bitmap()
{
    Release();
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

    mTileOrder = 0;
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

    mTileOrder = 0;
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

bool Bitmap::MakeTiled(Uint8 order)
{
    RT_ASSERT(order <= 16);

    const Uint32 bitsPerPixel = BitsPerPixel(mFormat);
    const Uint32 bytesPerPixel = bitsPerPixel / 8;

    const size_t dataSize = mWidth * mHeight * bitsPerPixel / 8;
    if (dataSize == 0)
    {
        RT_LOG_ERROR("Invalid bitmap format");
        return false;
    }

    // align to cache line
    Uint8* newData = (Uint8*)AlignedMalloc(dataSize + RT_CACHE_LINE_SIZE, RT_CACHE_LINE_SIZE);
    if (!newData)
    {
        RT_LOG_ERROR("Memory allocation failed");
        return false;
    }

    const Uint32 tileSize = 1 << order;
    const Uint32 bytesPerTileRow = bytesPerPixel << order;
    const Uint32 bytesPerTile = bytesPerPixel << (2 * order);
    const Uint32 numTilesX = mWidth >> order;
    const Uint32 numTilesY = mHeight >> order;

    for (Uint32 y = 0; y < numTilesY; ++y)
    {
        for (Uint32 x = 0; x < numTilesX; ++x)
        {
            for (Uint32 i = 0; i < tileSize; ++i)
            {
                const Uint32 textureX = x * tileSize;
                const Uint32 textureY = y * tileSize + i;

                const Uint8* srcData = mData + bytesPerPixel * (mWidth * textureY + textureX);
                Uint8* destData = newData + bytesPerTile * ((numTilesX * y) + x) + i * bytesPerTileRow;
                memcpy(destData, srcData, bytesPerTileRow);
            }
        }
    }

    // swap texture data
    AlignedFree(mData);
    mData = newData;
    mTileOrder = order;

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

    const Uint32 tileSize = 1 << mTileOrder;
    const Uint32 tileX = x >> mTileOrder;
    const Uint32 tileY = y >> mTileOrder;
    const Uint32 tilesInRow = mWidth >> mTileOrder;

    // calculate position inside tile
    const Uint32 tileMask = tileSize - 1;
    x &= tileMask;
    y &= tileMask;

    const Uint32 offset = tileSize * (tileSize * (tilesInRow * tileY + tileX) + y ) + x;

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

Vector4 Bitmap::Sample(Vector4 coords, const SamplerDesc& sampler) const
{
    // FPU version
    /*
    const Int32 intU = static_cast<Int32>(coords.x);
    const Int32 intV = static_cast<Int32>(coords.y);
    coords.x -= (Float)intU;
    coords.y -= (Float)intV;

    coords.x *= mWidth;
    coords.y *= mHeight;
    const Int32 u0 = static_cast<Int32>(coords.x);
    const Int32 v0 = static_cast<Int32>(coords.y);
    */

    //_MM_SET_ROUNDING_MODE(_MM_ROUND_DOWN);

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

    return GetPixel(u0, v0, sampler.forceLinearSpace);

    /*
    Int32 u1 = u0 + 1;
    Int32 v1 = v0 + 1;
    if (u1 >= mWidth) u1 = 0;
    if (v1 >= mHeight) v1 = 0;

    const Float weightU = coords.x - u0;
    const Float weightV = coords.y - v0;

    // TODO this is slowa
    const Vector4 value00 = GetPixel(u0, v0, sampler.forceLinearSpace);
    const Vector4 value01 = GetPixel(u0, v1, sampler.forceLinearSpace);
    const Vector4 value10 = GetPixel(u1, v0, sampler.forceLinearSpace);
    const Vector4 value11 = GetPixel(u1, v1, sampler.forceLinearSpace);

    // bilinear interpolation
    const Vector4 value0 = Vector4::Lerp(value00, value01, weightV);
    const Vector4 value1 = Vector4::Lerp(value10, value11, weightV);
    return Vector4::Lerp(value0, value1, weightU);
    */
}

} // namespace rt
