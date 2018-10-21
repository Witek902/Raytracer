#include "PCH.h"
#include "Bitmap.h"
#include "Logger.h"
#include "BlockCompression.h"
#include "Timer.h"

#include <string.h>

///////////////////////////////////////////////////////////////////////////

#define DDS_MAGIC_NUMBER    0x20534444

#define DDSD_CAPS   0x1
#define DDSD_HEIGHT 0x2
#define DDSD_WIDTH  0x4
#define DDSD_PITCH  0x8
#define DDSD_PIXELFORMAT    0x1000
#define DDSD_MIPMAPCOUNT    0x20000
#define DDSD_LINEARSIZE     0x80000
#define DDSD_DEPTH          0x800000

#define DDPF_ALPHAPIXELS        0x00000001
#define DDPF_ALPHA              0x00000002
#define DDPF_FOURCC             0x00000004
#define DDPF_PALETTEINDEXED8    0x00000020
#define DDPF_RGB                0x00000040
#define DDPF_LUMINANCE          0x00020000

#define ID_DXT1   0x31545844
#define ID_DXT3   0x33545844
#define ID_DXT5   0x35545844

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3) (((Uint32)(Uint8)(ch0) | ((Uint32)(Uint8)(ch1) << 8) | ((Uint32)(Uint8)(ch2) << 16) | ((Uint32)(Uint8)(ch3) << 24)))
#endif /* defined(MAKEFOURCC) */

#pragma pack(push, 2)

struct BitmapFileHeader
{
    Uint16 bfType;
    Uint32 bfSize;
    Uint16 bfReserved1;
    Uint16 bfReserved2;
    Uint32 bfOffBits;
};

struct BitmapInfoHeader
{
    Uint32 biSize;
    Uint32 biWidth;
    Uint32 biHeight;
    Uint16 biPlanes;
    Uint16 biBitCount;
    Uint32 biCompression;
    Uint32 biSizeImage;
    Uint32 biXPelsPerMeter;
    Uint32 biYPelsPerMeter;
    Uint32 biClrUsed;
    Uint32 biClrImportant;
};

struct BMPHeader
{
    BitmapFileHeader fileHeader;
    BitmapInfoHeader infoHeader;
};

#pragma pack(pop)

///////////////////////////////////////////////////////////////////////////

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
    case Format::R32G32B32_Float:       return 8 * 4 * 3;
    case Format::R32G32B32A32_Float:    return 8 * 4 * 4;
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

bool Bitmap::MakeTiled(Uint8 order)
{
    assert(order <= 16);

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
            RT_LOG_ERROR("Failed to load '%hs' - unknown format", path);
            fclose(file);
            return false;
        }
    }

    fclose(file);

    const float elapsedTime = static_cast<float>(1000.0 * timer.Stop());
    RT_LOG_INFO("Bitmap '%hs' loaded in %.3fms: format=%s, width=%u, height=%u", path, elapsedTime, FormatToString(mFormat), mWidth, mHeight);
    return true;
}

bool Bitmap::LoadBMP(FILE* file, const char* path)
{
    BitmapFileHeader fileHeader;
    if (fread(&fileHeader, sizeof(BitmapFileHeader), 1, file) != 1)
    {
        return false;
    }

    if (fileHeader.bfType != 0x4D42)
    {
        return false;
    }

    BitmapInfoHeader infoHeader;
    if (fread(&infoHeader, sizeof(BitmapInfoHeader), 1, file) != 1)
    {
        RT_LOG_ERROR("Failed to read BMP info header from file '%hs'", path);
        return false;
    }

    if (infoHeader.biPlanes != 1 || infoHeader.biCompression != 0)
    {
        RT_LOG_ERROR("Unsupported BMP format: '%hs'", path);
        return false;
    }

    Format format = Format::Unknown;
    if (infoHeader.biBitCount == 24)
    {
        format = Format::B8G8R8_Uint;
    }
    else if (infoHeader.biBitCount == 8)
    {
        format = Format::R8_Uint;
    }
    else
    {
        RT_LOG_ERROR("Unsupported BMP bit depth (%u): '%hs'", (Uint32)infoHeader.biBitCount, path);
        return false;
    }

    if (infoHeader.biWidth == 0 || infoHeader.biHeight == 0)
    {
        RT_LOG_ERROR("Invalid image size: '%hs'", path);
        return false;
    }

    if (!Init(infoHeader.biWidth, infoHeader.biHeight, format))
    {
        return false;
    }

    if (fseek(file, fileHeader.bfOffBits, SEEK_SET) != 0)
    {
        RT_LOG_ERROR("Failed to read bitmap data from file '%hs'", path);
        return false;
    }

    const size_t dataSize = GetDataSize(infoHeader.biWidth, infoHeader.biHeight, format);
    if (fread(mData, dataSize, 1, file) != 1)
    {
        RT_LOG_ERROR("Failed to read bitmap data from file '%hs'", path);
        return false;
    }

    return true;
}

bool Bitmap::SaveBMP(const char* path, bool flipVertically) const
{
    Uint32 dataSize = 3 * mWidth * mHeight;

    std::vector<Uint8> tmpData(dataSize);
    if (mFormat == Format::B8G8R8A8_Uint)
    {
        const Uint8* data = reinterpret_cast<const Uint8*>(mData);

        Uint32 i = 0;
        for (Uint32 y = 0; y < (Uint32)mHeight; ++y)
        {
            const Uint32 realY = flipVertically ? mHeight - 1 - y : y;
            for (Uint32 x = 0; x < (Uint32)mWidth; ++x)
            {
                const Uint32 p = mWidth * realY + x;
                tmpData[i++] = data[4 * p];
                tmpData[i++] = data[4 * p + 1];
                tmpData[i++] = data[4 * p + 2];
            }
        }
    }
    else
    {
        RT_LOG_ERROR("Bitmap::SaveBMP: Unsupported format");
        return false;
    }

    const BMPHeader header =
    {
        // BitmapFileHeader
        {
            /* bfType */        0x4D42,
            /* bfSize */        static_cast<Uint32>(sizeof(BMPHeader) + dataSize),
            /* bfReserved1 */   0,
            /* bfReserved2 */   0,
            /* bfOffBits */     sizeof(BMPHeader),
        },

        // BitmapInfoHeader
        {
            sizeof(BitmapInfoHeader),
            mWidth,
            mHeight,
            1,
            24,
            0, // BI_RGB
            dataSize,
            96, 96, 0, 0
        },
    };

    FILE* file = fopen(path, "wb");
    if (!file)
    {
        RT_LOG_ERROR("Failed to open target image '%s', code = %u", path, stderr);
        return false;
    }

    if (fwrite(&header, sizeof(BMPHeader), 1, file) != 1)
    {
        RT_LOG_ERROR("Failed to write bitmap header to file '%s', code = %u", path, stderr);
        fclose(file);
        return false;
    }

    if (fwrite(tmpData.data(), dataSize, 1, file) != 1)
    {
        RT_LOG_ERROR("Failed to write bitmap image data to file '%s', code = %u", path, stderr);
        fclose(file);
        return false;
    }

    RT_LOG_INFO("Image file '%s' written successfully", path);
    fclose(file);
    return true;
}

bool Bitmap::LoadDDS(FILE* file, const char* path)
{
    struct DDS_PIXELFORMAT
    {
        Uint32 dwSize;
        Uint32 dwFlags;
        Uint32 dwFourCC;
        Uint32 dwRGBBitCount;
        Uint32 dwRBitMask;
        Uint32 dwGBitMask;
        Uint32 dwBBitMask;
        Uint32 dwAlphaBitMask;
    };

    struct Header
    {
        Uint32 dwMagic;
        Uint32 dwSize;
        Uint32 dwFlags;
        Uint32 dwHeight;
        Uint32 dwWidth;
        Uint32 dwPitchOrLinearSize;
        Uint32 dwDepth;
        Uint32 dwMipMapCount;
        Uint32 dwReserved1[11];

        //  DDPIXELFORMAT
        DDS_PIXELFORMAT sPixelFormat;

        //  DDCAPS2
        struct
        {
            Uint32 dwCaps1;
            Uint32 dwCaps2;
            Uint32 dwDDSX;
            Uint32 dwReserved;
        } sCaps;

        Uint32 dwReserved2;
    };

    struct HeaderDX10
    {
        Uint32 dxgiFormat;
        Uint32 resourceDimension;
        Uint32 miscFlag;
        Uint32 arraySize;
        Uint32 miscFlags2;
    };

    // read header
    Header header;
    if (fread(&header, sizeof(header), 1, file) != 1)
    {
        return false;
    }

    //check magic number
    if (header.dwMagic != DDS_MAGIC_NUMBER)
    {
        return false;
    }

    Uint32 width = header.dwWidth;
    Uint32 height = header.dwHeight;
    if (width < 1 || height < 1 || width >= std::numeric_limits<Uint16>::max() || height >= std::numeric_limits<Uint16>::max())
    {
        RT_LOG_ERROR("Unsupported DDS format in file '%hs'", path);
        return false;
    }

    bool linearSpace = false;
    Format format = Format::Unknown;
    const DDS_PIXELFORMAT& pf = header.sPixelFormat;
    if (pf.dwFlags & DDPF_RGB)
    {
        if (pf.dwRBitMask == 0xFF && pf.dwGBitMask == 0xFF && pf.dwBBitMask == 0xFF && pf.dwAlphaBitMask == 0xFF)
        {
            format = Format::B8G8R8A8_Uint; // DXGI_FORMAT_R8G8B8A8_UNORM;
        }
        if (pf.dwRBitMask == 0xFFFFFFFF && pf.dwGBitMask == 0 && pf.dwBBitMask == 0 && pf.dwAlphaBitMask == 0)
        {
            // format = Format::R32_Float; // DXGI_FORMAT_R32_FLOAT;
        }
    }
    else if (pf.dwFlags & DDPF_FOURCC)
    {
        if (pf.dwFourCC == 113) // D3DFMT_A16B16G16R16F
        {
            // format = Format::R16G16B16A16_Float;
        }
        else if (pf.dwFourCC == 114) // DXGI_FORMAT_R32_FLOAT
        {
            // format = Format::R32_Float;
        }
        else if (pf.dwFourCC == 116) // D3DFMT_A32B32G32R32F
        {
            format = Format::R32G32B32A32_Float;
            linearSpace = true;
        }
        else if (MAKEFOURCC('D', 'X', 'T', '1') == pf.dwFourCC)
            format = Format::BC1; //DXGI_FORMAT_BC1_UNORM
        else if (MAKEFOURCC('A', 'T', 'I', '1') == pf.dwFourCC)
            format = Format::BC4; //DXGI_FORMAT_BC4_UNORM
        else if (MAKEFOURCC('B', 'C', '4', 'U') == pf.dwFourCC)
            format = Format::BC4; //DXGI_FORMAT_BC4_UNORM
        else if (MAKEFOURCC('B', 'C', '4', 'S') == pf.dwFourCC)
            format = Format::BC4; //DXGI_FORMAT_BC4_SNORM
        else if (MAKEFOURCC('A', 'T', 'I', '2') == pf.dwFourCC)
            format = Format::BC5; //DXGI_FORMAT_BC5_UNORM
        else if (MAKEFOURCC('B', 'C', '5', 'U') == pf.dwFourCC)
            format = Format::BC5; //DXGI_FORMAT_BC5_UNORM
        else if (MAKEFOURCC('D', 'X', '1', '0') == pf.dwFourCC)
        {
            // read DX10 header
            HeaderDX10 headerDX10;
            if (fread(&headerDX10, sizeof(headerDX10), 1, file) != 1)
            {
                RT_LOG_ERROR("Failed to read DX10 header '%hs'", path);
                return false;
            }

            if (headerDX10.dxgiFormat == 2) // DXGI_FORMAT_R32G32B32A32_FLOAT
            {
                format = Format::R32G32B32A32_Float;
                linearSpace = true;
            }
            else if (headerDX10.dxgiFormat == 87) // DXGI_FORMAT_B8G8R8A8_UNORM
            {
                format = Format::B8G8R8A8_Uint;
                linearSpace = true;
            }
        }
    }

    if (format == Format::Unknown)
    {
        RT_LOG_ERROR("Unsupported DDS format in file '%hs'", path);
        return false;
    }

    if (!Init(width, height, format, nullptr, linearSpace))
    {
        return false;
    }

    // TODO support for DXT textures that has dimension non-4-multiply
    const size_t dataSize = GetDataSize(width, height, format);
    if (fread(mData, dataSize, 1, file) != 1)
    {
        RT_LOG_ERROR("Failed to read bitmap data from file '%hs', errno = %u", path, errno);
        return false;
    }

    return true;
}

void Bitmap::SetPixel(Uint32 x, Uint32 y, const Vector4& value)
{
    const Uint32 offset = mWidth * y + x;

    switch (mFormat)
    {
        case Format::B8G8R8A8_Uint:
        {
            Uint8* target = mData + (4 * offset);
            const Vector4 clampedValue = value.Swizzle<2, 1, 0, 3>() * 255.0f;
            clampedValue.Store4_NonTemporal(target);
            break;
        }

        case Format::R32G32B32_Float:
        {
            float* target = reinterpret_cast<float*>(mData) + 3 * offset;
            target[0] = value[0];
            target[1] = value[1];
            target[2] = value[2];
            break;
        }

        case Format::R32G32B32A32_Float:
        {
            float* target = reinterpret_cast<float*>(mData) + 4 * offset;
            _mm_stream_ps(target, value);
            //Vector4* target = reinterpret_cast<Vector4*>(mData) + offset;
            //*target = value;
            break;
        }

        default:
            RT_FATAL("Unsupported bitmap format");
    }
}

Vector4 Bitmap::GetPixel(Uint32 x, Uint32 y, const bool forceLinearSpace) const
{
    assert(x < mWidth);
    assert(y < mHeight);

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
            const float* source = reinterpret_cast<float*>(mData) + 3 * offset;
            color = Vector4(source[0], source[1], source[2], 0.0f);
            break;
        }

        case Format::R32G32B32A32_Float:
        {
            const Vector4* source = reinterpret_cast<Vector4*>(mData) + offset;
            RT_PREFETCH_L2(source - mWidth);
            RT_PREFETCH_L2(source + mWidth);
            color = *source;
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
            RT_FATAL("Unsupported bitmap format");
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

void Bitmap::AccumulateFloat_Unsafe(const Uint32 x, const Uint32 y, const math::Vector4 value)
{
    assert(mFormat == Format::R32G32B32A32_Float);

    Vector4* typedData = reinterpret_cast<Vector4*>(mData);
    typedData[mWidth * y + x] += value;
}

void Bitmap::Zero()
{
    if (mData)
    {
        memset(mData, 0, GetDataSize(mWidth, mHeight, mFormat));
    }
}

} // namespace rt
