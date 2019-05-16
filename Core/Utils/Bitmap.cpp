#include "PCH.h"
#include "Bitmap.h"
#include "Logger.h"
#include "BlockCompression.h"
#include "Timer.h"

namespace rt {

using namespace math;

static_assert(sizeof(Bitmap) <= 64, "Bitmap class is too big");

Uint32 Bitmap::BitsPerPixel(Format format)
{
    switch (format)
    {
    case Format::Unknown:               return 0;
    case Format::R8_UNorm:              return 8 * sizeof(Uint8);
    case Format::R8G8_UNorm:            return 8 * sizeof(Uint8) * 2;
    case Format::B8G8R8_UNorm:          return 8 * sizeof(Uint8) * 3;
    case Format::B8G8R8A8_UNorm:        return 8 * sizeof(Uint8) * 4;
    case Format::R16_UNorm:             return 8 * sizeof(Uint16);
    case Format::R16G16_UNorm:          return 8 * sizeof(Uint16) * 2;
    case Format::R16G16B16A16_UNorm:    return 8 * sizeof(Uint16) * 4;
    case Format::R32_Float:             return 8 * sizeof(float);
    case Format::R32G32B32_Float:       return 8 * sizeof(float) * 3;
    case Format::R32G32B32A32_Float:    return 8 * sizeof(float) * 4;
    case Format::R16_Half:              return 8 * sizeof(Half) * 1;
    case Format::R16G16_Half:           return 8 * sizeof(Half) * 2;
    case Format::R16G16B16_Half:        return 8 * sizeof(Half) * 3;
    case Format::R16G16B16A16_Half:     return 8 * sizeof(Half) * 4;
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
    case Format::R8_UNorm:              return "R8_UNorm";
    case Format::R8G8_UNorm:            return "R8G8_UNorm";
    case Format::B8G8R8_UNorm:          return "B8G8R8_UNorm";
    case Format::B8G8R8A8_UNorm:        return "B8G8R8A8_UNorm";
    case Format::R16_UNorm:             return "R16_UNorm";
    case Format::R16G16_UNorm:          return "R16G16_UNorm";
    case Format::R16G16B16A16_UNorm:    return "R16G16B16A16_UNorm";
    case Format::R32_Float:             return "R32_Float";
    case Format::R32G32B32_Float:       return "R32G32B32_Float";
    case Format::R32G32B32A32_Float:    return "R32G32B32A32_Float";
    case Format::R16_Half:              return "R16_Half";
    case Format::R16G16_Half:           return "R16G16_Half";
    case Format::R16G16B16_Half:        return "R16G16B16_Half";
    case Format::R16G16B16A16_Half:     return "R16G16B16A16_Half";
    case Format::BC1:                   return "BC1";
    case Format::BC4:                   return "BC4";
    case Format::BC5:                   return "BC5";
    }

    return "<unknown>";
}

size_t Bitmap::ComputeDataSize(Uint32 width, Uint32 height, Format format)
{
    const Uint64 dataSize = (Uint64)height * (Uint64)ComputeDataStride(width, format);

    if (dataSize >= (Uint64)std::numeric_limits<size_t>::max())
    {
        return std::numeric_limits<size_t>::max();
    }

    return (size_t)dataSize;
}

Uint32 Bitmap::ComputeDataStride(Uint32 width, Format format)
{
    return width * (Uint64)BitsPerPixel(format) / 8;
}

Bitmap::Bitmap(const char* debugName)
    : mData(nullptr)
    , mWidth(0)
    , mHeight(0)
    , mStride(0)
    , mFormat(Format::Unknown)
    , mLinearSpace(false)
{
    RT_ASSERT(debugName, "Invalid debug name");
    mDebugName = _strdup(debugName);
}

Bitmap::~Bitmap()
{
    RT_LOG_INFO("Releasing bitmap '%s'...", mDebugName);
    free(mDebugName);

    Release();
}

Bitmap::Bitmap(Bitmap&&) = default;

Bitmap& Bitmap::operator = (Bitmap&&) = default;

void Bitmap::Clear()
{
    if (mData)
    {
        memset(mData, 0, ComputeDataSize(mWidth, mHeight, mFormat));
    }
}

void Bitmap::Release()
{
    if (mData)
    {
        AlignedFree(mData);
        mData = nullptr;
    }

    mStride = 0;
    mWidth = 0;
    mHeight = 0;
    mFormat = Format::Unknown;
}

bool Bitmap::Init(Uint32 width, Uint32 height, Format format, const void* data, bool linearSpace)
{
    const size_t dataSize = ComputeDataSize(width, height, format);
    if (dataSize == 0)
    {
        RT_LOG_ERROR("Invalid bitmap format");
        return false;
    }
    if (dataSize == std::numeric_limits<size_t>::max())
    {
        RT_LOG_ERROR("Bitmap is too big");
        return false;
    }

    Release();

    // align to cache line
    const Uint32 marigin = RT_CACHE_LINE_SIZE;
    mData = (Uint8*)AlignedMalloc(dataSize + marigin, RT_CACHE_LINE_SIZE);
    if (!mData)
    {
        RT_LOG_ERROR("Bitmap: Memory allocation failed");
        return false;
    }

    if (data)
    {
        memcpy(mData, data, dataSize);
    }

    mStride = ComputeDataStride(width, format);
    mWidth = width;
    mHeight = height;
    mSize = VectorInt4(width, height, width, height);
    mFloatSize = mSize.ConvertToFloat();
    mFormat = format;
    mLinearSpace = linearSpace;

    return true;
}

bool Bitmap::Copy(Bitmap& target, const Bitmap& source)
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

    memcpy(target.GetData(), source.GetData(), ComputeDataSize(target.mWidth, target.mHeight, target.mFormat));
    return true;
}

bool Bitmap::Load(const char* path)
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

    const size_t rowOffset = static_cast<size_t>(mStride) * static_cast<size_t>(y);
    const Uint8* rowData = mData + rowOffset;

    Vector4 color;
    switch (mFormat)
    {
    case Format::R8_UNorm:
    {
        const Uint32 value = rowData[x];
        color = Vector4::FromInteger(value) * (1.0f / 255.0f);
        break;
    }

    case Format::R8G8_UNorm:
    {
        color = Vector4::Load_2xUint8_Norm(rowData + 2u * x);
        break;
    }

    case Format::B8G8R8_UNorm:
    {
        const Uint8* source = rowData + 3u * x;
        color = Vector4::LoadBGR_UNorm(source);
        break;
    }

    case Format::B8G8R8A8_UNorm:
    {
        const Uint8* source = rowData + 4u * x;
        color = Vector4::Load_4xUint8(source).Swizzle<2, 1, 0, 3>() * (1.0f / 255.0f);
        break;
    }

    case Format::R16_UNorm:
    {
        const Uint16* source = reinterpret_cast<const Uint16*>(rowData) + x;
        color = Vector4::FromInteger(*source) * (1.0f / 65535.0f);
        break;
    }

    case Format::R16G16_UNorm:
    {
        const Uint16* source = reinterpret_cast<const Uint16*>(rowData) + 2u * x;
        color = Vector4::Load_2xUint16_Norm(source);
        break;
    }

    case Format::R16G16B16A16_UNorm:
    {
        const Uint16* source = reinterpret_cast<const Uint16*>(rowData) + 4u * x;
        color = Vector4::Load_4xUint16(source) * (1.0f / 65535.0f);
        break;
    }

    case Format::R32_Float:
    {
        const float* source = reinterpret_cast<const float*>(rowData) + x;
        color = Vector4(*source);
        break;
    }

    case Format::R32G32B32_Float:
    {
        const float* source = reinterpret_cast<const float*>(rowData) + 3u * x;
        color = Vector4(source) & Vector4::MakeMask<1, 1, 1, 0>();
        break;
    }

    case Format::R32G32B32A32_Float:
    {
        const Vector4* source = reinterpret_cast<const Vector4*>(rowData) + x;
        RT_PREFETCH_L2(source - mWidth);
        RT_PREFETCH_L2(source + mWidth);
        color = *source;
        break;
    }

    case Format::R16_Half:
    {
        const Half* source = reinterpret_cast<const Half*>(rowData) + x;
        color = Vector4(ConvertHalfToFloat(*source));
        break;
    }

    case Format::R16G16_Half:
    {
        const Half* source = reinterpret_cast<const Half*>(rowData) + 2u * x;
        color = Vector4::FromHalves(source) & Vector4::MakeMask<1, 1, 0, 0>();
        break;
    }

    case Format::R16G16B16_Half:
    {
        const Half* source = reinterpret_cast<const Half*>(rowData) + 3u * x;
        color = Vector4::FromHalves(source) & Vector4::MakeMask<1, 1, 1, 0>();
        break;
    }

    case Format::R16G16B16A16_Half:
    {
        const Half* source = reinterpret_cast<const Half*>(rowData) + 4u * x;
        color = Vector4::FromHalves(source);
        break;
    }

    case Format::BC1:
    {
        color = DecodeBC1(reinterpret_cast<const Uint8*>(rowData), x, y, mWidth);
        break;
    }

    case Format::BC4:
    {
        color = DecodeBC4(reinterpret_cast<const Uint8*>(rowData), x, y, mWidth);
        break;
    }

    case Format::BC5:
    {
        color = DecodeBC5(reinterpret_cast<const Uint8*>(rowData), x, y, mWidth);
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

RT_FORCE_NOINLINE
void Bitmap::GetPixelBlock(const VectorInt4 coords, const bool forceLinearSpace, Vector4* outColors) const
{
    RT_ASSERT(coords.x >= 0 && coords.x < (Int32)mWidth);
    RT_ASSERT(coords.y >= 0 && coords.y < (Int32)mHeight);
    RT_ASSERT(coords.z >= 0 && coords.z < (Int32)mWidth);
    RT_ASSERT(coords.w >= 0 && coords.w < (Int32)mHeight);

    // calculate offsets in pixels array for each corner
    VectorInt4 offsets = coords.Swizzle<1, 1, 3, 3>() * (Int32)mWidth + coords.Swizzle<0, 2, 0, 2>();

    Vector4 color[4];

    switch (mFormat)
    {
    case Format::R8_UNorm:
    {
        constexpr float scale = 1.0f / 255.0f;
        const Uint32 value0 = mData[(Uint32)offsets.x];
        const Uint32 value1 = mData[(Uint32)offsets.y];
        const Uint32 value2 = mData[(Uint32)offsets.z];
        const Uint32 value3 = mData[(Uint32)offsets.w];
        const Vector4 values = Vector4::FromIntegers(value0, value1, value2, value3) * scale;
        color[0] = values.SplatX();
        color[1] = values.SplatY();
        color[2] = values.SplatZ();
        color[3] = values.SplatW();
        break;
    }

    case Format::R8G8_UNorm:
    {
        offsets <<= 1; // offsets *= 2
        color[0] = Vector4::Load_2xUint8_Norm(mData + (Uint32)offsets.x);
        color[1] = Vector4::Load_2xUint8_Norm(mData + (Uint32)offsets.y);
        color[2] = Vector4::Load_2xUint8_Norm(mData + (Uint32)offsets.z);
        color[3] = Vector4::Load_2xUint8_Norm(mData + (Uint32)offsets.w);
        break;
    }

    case Format::B8G8R8_UNorm:
    {
        offsets += (offsets << 1); // offsets *= 3
        color[0] = Vector4::LoadBGR_UNorm(mData + (Uint32)offsets.x);
        color[1] = Vector4::LoadBGR_UNorm(mData + (Uint32)offsets.y);
        color[2] = Vector4::LoadBGR_UNorm(mData + (Uint32)offsets.z);
        color[3] = Vector4::LoadBGR_UNorm(mData + (Uint32)offsets.w);
        break;
    }

    case Format::B8G8R8A8_UNorm:
    {
        constexpr float scale = 1.0f / 255.0f;
        offsets <<= 2; // offsets *= 4
        color[0] = Vector4::Load_4xUint8(mData + (Uint32)offsets.x).Swizzle<2, 1, 0, 3>() * scale;
        color[1] = Vector4::Load_4xUint8(mData + (Uint32)offsets.y).Swizzle<2, 1, 0, 3>() * scale;
        color[2] = Vector4::Load_4xUint8(mData + (Uint32)offsets.z).Swizzle<2, 1, 0, 3>() * scale;
        color[3] = Vector4::Load_4xUint8(mData + (Uint32)offsets.w).Swizzle<2, 1, 0, 3>() * scale;
        break;
    }

    case Format::R16_UNorm:
    {
        constexpr float scale = 1.0f / 65535.0f;
        const Uint32 value0 = reinterpret_cast<const Uint16*>(mData)[(Uint32)offsets.x];
        const Uint32 value1 = reinterpret_cast<const Uint16*>(mData)[(Uint32)offsets.y];
        const Uint32 value2 = reinterpret_cast<const Uint16*>(mData)[(Uint32)offsets.z];
        const Uint32 value3 = reinterpret_cast<const Uint16*>(mData)[(Uint32)offsets.w];
        const Vector4 values = Vector4::FromIntegers(value0, value1, value2, value3) * scale;
        color[0] = values.SplatX();
        color[1] = values.SplatY();
        color[2] = values.SplatZ();
        color[3] = values.SplatW();
        break;
    }

    case Format::R16G16_UNorm:
    {
        offsets <<= 1; // offsets *= 2
        const Uint16* source0 = reinterpret_cast<const Uint16*>(mData) + (Uint32)offsets.x;
        const Uint16* source1 = reinterpret_cast<const Uint16*>(mData) + (Uint32)offsets.y;
        const Uint16* source2 = reinterpret_cast<const Uint16*>(mData) + (Uint32)offsets.z;
        const Uint16* source3 = reinterpret_cast<const Uint16*>(mData) + (Uint32)offsets.w;
        color[0] = Vector4::Load_2xUint16_Norm(source0);
        color[1] = Vector4::Load_2xUint16_Norm(source1);
        color[2] = Vector4::Load_2xUint16_Norm(source2);
        color[3] = Vector4::Load_2xUint16_Norm(source3);
        break;
    }

    case Format::R16G16B16A16_UNorm:
    {
        constexpr float scale = 1.0f / 65535.0f;
        offsets <<= 2; // offsets *= 4
        const Uint16* source0 = reinterpret_cast<const Uint16*>(mData) + (Uint32)offsets.x;
        const Uint16* source1 = reinterpret_cast<const Uint16*>(mData) + (Uint32)offsets.y;
        const Uint16* source2 = reinterpret_cast<const Uint16*>(mData) + (Uint32)offsets.z;
        const Uint16* source3 = reinterpret_cast<const Uint16*>(mData) + (Uint32)offsets.w;
        color[0] = Vector4::Load_4xUint16(source0) * scale;
        color[1] = Vector4::Load_4xUint16(source1) * scale;
        color[2] = Vector4::Load_4xUint16(source2) * scale;
        color[3] = Vector4::Load_4xUint16(source3) * scale;
        break;
    }

    case Format::R32_Float:
    {
        const float* source0 = reinterpret_cast<const float*>(mData) + (Uint32)offsets.x;
        const float* source1 = reinterpret_cast<const float*>(mData) + (Uint32)offsets.y;
        const float* source2 = reinterpret_cast<const float*>(mData) + (Uint32)offsets.z;
        const float* source3 = reinterpret_cast<const float*>(mData) + (Uint32)offsets.w;
        color[0] = Vector4(*source0);
        color[1] = Vector4(*source1);
        color[2] = Vector4(*source2);
        color[3] = Vector4(*source3);
        break;
    }

    case Format::R32G32B32_Float:
    {
        offsets += (offsets << 1); // offsets *= 3
        const float* source0 = reinterpret_cast<const float*>(mData) + (Uint32)offsets.x;
        const float* source1 = reinterpret_cast<const float*>(mData) + (Uint32)offsets.y;
        const float* source2 = reinterpret_cast<const float*>(mData) + (Uint32)offsets.z;
        const float* source3 = reinterpret_cast<const float*>(mData) + (Uint32)offsets.w;
        color[0] = Vector4(source0) & Vector4::MakeMask<1, 1, 1, 0>();
        color[1] = Vector4(source1) & Vector4::MakeMask<1, 1, 1, 0>();
        color[2] = Vector4(source2) & Vector4::MakeMask<1, 1, 1, 0>();
        color[3] = Vector4(source3) & Vector4::MakeMask<1, 1, 1, 0>();
        break;
    }

    case Format::R32G32B32A32_Float:
    {
        color[0] = reinterpret_cast<const Vector4*>(mData)[offsets.x];
        color[1] = reinterpret_cast<const Vector4*>(mData)[offsets.y];
        color[2] = reinterpret_cast<const Vector4*>(mData)[offsets.z];
        color[3] = reinterpret_cast<const Vector4*>(mData)[offsets.w];
        break;
    }

    case Format::R16_Half:
    {
        const Half* source0 = reinterpret_cast<const Half*>(mData) + (Uint32)offsets.x;
        const Half* source1 = reinterpret_cast<const Half*>(mData) + (Uint32)offsets.y;
        const Half* source2 = reinterpret_cast<const Half*>(mData) + (Uint32)offsets.z;
        const Half* source3 = reinterpret_cast<const Half*>(mData) + (Uint32)offsets.w;
        color[0] = Vector4(ConvertHalfToFloat(*source0));
        color[1] = Vector4(ConvertHalfToFloat(*source1));
        color[2] = Vector4(ConvertHalfToFloat(*source2));
        color[3] = Vector4(ConvertHalfToFloat(*source3));
        break;
    }

    case Format::R16G16_Half:
    {
        offsets <<= 1; // offsets *= 2
        const Half* source0 = reinterpret_cast<const Half*>(mData) + (Uint32)offsets.x;
        const Half* source1 = reinterpret_cast<const Half*>(mData) + (Uint32)offsets.y;
        const Half* source2 = reinterpret_cast<const Half*>(mData) + (Uint32)offsets.z;
        const Half* source3 = reinterpret_cast<const Half*>(mData) + (Uint32)offsets.w;
        color[0] = Vector4::FromHalves(source0) & Vector4::MakeMask<1, 1, 0, 0>();
        color[1] = Vector4::FromHalves(source1) & Vector4::MakeMask<1, 1, 0, 0>();
        color[2] = Vector4::FromHalves(source2) & Vector4::MakeMask<1, 1, 0, 0>();
        color[3] = Vector4::FromHalves(source3) & Vector4::MakeMask<1, 1, 0, 0>();
        break;
    }

    case Format::R16G16B16_Half:
    {
        offsets += (offsets << 1); // offsets *= 3
        const Half* source0 = reinterpret_cast<const Half*>(mData) + (Uint32)offsets.x;
        const Half* source1 = reinterpret_cast<const Half*>(mData) + (Uint32)offsets.y;
        const Half* source2 = reinterpret_cast<const Half*>(mData) + (Uint32)offsets.z;
        const Half* source3 = reinterpret_cast<const Half*>(mData) + (Uint32)offsets.w;
        color[0] = Vector4::FromHalves(source0) & Vector4::MakeMask<1, 1, 1, 0>();
        color[1] = Vector4::FromHalves(source1) & Vector4::MakeMask<1, 1, 1, 0>();
        color[2] = Vector4::FromHalves(source2) & Vector4::MakeMask<1, 1, 1, 0>();
        color[3] = Vector4::FromHalves(source3) & Vector4::MakeMask<1, 1, 1, 0>();
        break;
    }

    case Format::R16G16B16A16_Half:
    {
        offsets <<= 2; // offsets *= 4
        const Half* source0 = reinterpret_cast<const Half*>(mData) + (Uint32)offsets.x;
        const Half* source1 = reinterpret_cast<const Half*>(mData) + (Uint32)offsets.y;
        const Half* source2 = reinterpret_cast<const Half*>(mData) + (Uint32)offsets.z;
        const Half* source3 = reinterpret_cast<const Half*>(mData) + (Uint32)offsets.w;
        color[0] = Vector4::FromHalves(source0);
        color[1] = Vector4::FromHalves(source1);
        color[2] = Vector4::FromHalves(source2);
        color[3] = Vector4::FromHalves(source3);
        break;
    }

    case Format::BC1:
    {
        color[0] = DecodeBC1(mData, coords.x, coords.y, mWidth);
        color[1] = DecodeBC1(mData, coords.z, coords.y, mWidth);
        color[2] = DecodeBC1(mData, coords.x, coords.w, mWidth);
        color[3] = DecodeBC1(mData, coords.z, coords.w, mWidth);
        break;
    }

    case Format::BC4:
    {
        color[0] = DecodeBC4(mData, coords.x, coords.y, mWidth);
        color[1] = DecodeBC4(mData, coords.z, coords.y, mWidth);
        color[2] = DecodeBC4(mData, coords.x, coords.w, mWidth);
        color[3] = DecodeBC4(mData, coords.z, coords.w, mWidth);
        break;
    }

    case Format::BC5:
    {
        color[0] = DecodeBC5(mData, coords.x, coords.y, mWidth);
        color[1] = DecodeBC5(mData, coords.z, coords.y, mWidth);
        color[2] = DecodeBC5(mData, coords.x, coords.w, mWidth);
        color[3] = DecodeBC5(mData, coords.z, coords.w, mWidth);
        break;
    }

    default:
    {
        RT_FATAL("Unsupported bitmap format");
    }
    }

    if (!mLinearSpace && !forceLinearSpace)
    {
        color[0] *= color[0];
        color[1] *= color[1];
        color[2] *= color[2];
        color[3] *= color[3];
    }

    outColors[0] = color[0];
    outColors[1] = color[1];
    outColors[2] = color[2];
    outColors[3] = color[3];
}

// performs 1D box blur in linear time
RT_FORCE_NOINLINE
static void BoxBlur_Internal(Vector4* __restrict targetLine, const Vector4* __restrict srcLine, const Uint32 radius, const Uint32 width)
{
    const float factor = 1.0f / (float)(2 * radius + 1);

    const Vector4* __restrict srcLineBegin = srcLine;
    const Vector4* __restrict srcLineEnd = srcLine;

    const Vector4 firstValue = srcLine[0];
    const Vector4 lastValue = srcLine[width - 1];
    Vector4 val = firstValue * static_cast<float>(radius + 1);

    for (Uint32 j = 0; j < radius; j++)
    {
        val += *(srcLineBegin++);
    }

    for (Uint32 j = 0; j <= radius; j++)
    {
        val += *(srcLineBegin++) - firstValue;
        *(targetLine++) = val * factor;
    }

    for (Uint32 j = radius + 1; j < width - radius; j++)
    {
        val += *(srcLineBegin++) - *(srcLineEnd++);
        *(targetLine++) = val * factor;
    }

    for (Uint32 j = width - radius; j < width; j++)
    {
        val += lastValue - *(srcLineEnd++);
        *(targetLine++) = val * factor;
    }
}

bool Bitmap::GaussianBlur(const float sigma, const Uint32 n)
{
    if (mFormat != Format::R32G32B32_Float)
    {
        RT_LOG_ERROR("GaussianBlur: Unsupported texture format");
        return false;
    }

    // TODO get rid of this
    const Uint32 MaxLineSize = 4096;
    if (mWidth > MaxLineSize || mHeight > MaxLineSize)
    {
        RT_LOG_ERROR("GaussianBlur: Image too big");
        return false;
    }

    // based on http://blog.ivank.net/fastest-gaussian-blur.html

    float wIdeal = sqrtf((12.0f * sigma * sigma / n) + 1.0f);
    Uint32 wl = (Uint32)floorf(wIdeal);
    if (wl % 2 == 0)
    {
        wl--;
    }

    const Uint32 wu = wl + 2;
    const float mIdeal = (12.0f * sigma * sigma - n * wl * wl - 4.0f * n * wl - 3.0f * n) / (-4.0f * wl - 4.0f);
    const float m = roundf(mIdeal);

    const Uint32 numColumns = 6; // RT_CACHE_LINE_SIZE / sizeof(Float3);

    Vector4 tempLineA[numColumns][MaxLineSize];
    Vector4 tempLineB[numColumns][MaxLineSize];

    Vector4* sourceLinePtr = nullptr;
    Vector4* targetLinePtr = nullptr;

    // horizontal blur
    for (Uint32 y = 0; y < mHeight; ++y)
    {
        Float3* rowPtr = &GetPixelRef<Float3>(0, y);

        for (Uint32 x = 0; x < mWidth; ++x)
        {
            tempLineB[0][x] = Vector4((float*)(rowPtr + x));
        }

        sourceLinePtr = tempLineB[0];
        targetLinePtr = tempLineA[0];

        for (Uint32 i = 0; i < n; ++i)
        {
            const Uint32 radius = i < m ? wl : wu;
            BoxBlur_Internal(targetLinePtr, sourceLinePtr, radius, mWidth);
            std::swap(sourceLinePtr, targetLinePtr);
        }

        for (Uint32 x = 0; x < mWidth; ++x)
        {
            *(rowPtr + x) = targetLinePtr[x].ToFloat3();
        }
    }

    // vertical blur
    for (Uint32 x = 0; x < mWidth; x += numColumns)
    {
        for (Uint32 y = 0; y < mHeight; ++y)
        {
            for (Uint32 i = 0; i < numColumns; ++i)
            {
                tempLineA[i][y] = Vector4(GetPixelRef<Float3>(x + i, y));
            }
        }

        for (Uint32 i = 0; i < numColumns; ++i)
        {
            sourceLinePtr = tempLineA[i];
            targetLinePtr = tempLineB[i];

            for (Uint32 j = 0; j < n; ++j)
            {
                const Uint32 radius = j < m ? wl : wu;
                BoxBlur_Internal(targetLinePtr, sourceLinePtr, radius, mHeight);
                std::swap(sourceLinePtr, targetLinePtr);
            }
        }


        for (Uint32 y = 0; y < mHeight; ++y)
        {
            for (Uint32 i = 0; i < numColumns; ++i)
            {
                GetPixelRef<Float3>(x + i, y) = tempLineA[i][y].ToFloat3();
            }
        }
    }

    return true;
}

} // namespace rt
