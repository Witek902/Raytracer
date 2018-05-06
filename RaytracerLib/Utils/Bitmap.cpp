#include "PCH.h"
#include "Bitmap.h"
#include "Logger.h"

#include <cassert>

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


///////////////////////////////////////////////////////////////////////////

namespace rt {

using namespace math;

size_t Bitmap::BitsPerPixel(Format format)
{
    switch (format)
    {
    case Format::Unknown:               return 0;
    case Format::B8G8R8_Uint:           return 8 * 3;
    case Format::B8G8R8A8_Uint:         return 8 * 4;
    case Format::R32G32B32_Float:       return 8 * 4 * 3;
    case Format::R32G32B32A32_Float:    return 8 * 4 * 4;
    }

    return 0;
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
        _aligned_free(mData);
        mData = nullptr;
    }

    mWidth = 0;
    mHeight = 0;
    mFormat = Format::Unknown;
}

Bool Bitmap::Init(Uint32 width, Uint32 height, Format format, const void* data, bool linearSpace)
{
    if (width > MaxSize)
    {
        RT_LOG_ERROR("Bitmap width is too large");
        return false;
    }

    if (height > MaxSize)
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
    const Uint32 marigin = 16;
    mData = _aligned_malloc(dataSize + marigin, RT_CACHE_LINE_SIZE);
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
    mLinearSpace = linearSpace;

    return true;
}

Bool Bitmap::Load(const char* path)
{
    FILE* file = fopen(path, "rb");
    if (!file)
    {
        RT_LOG_ERROR("Failed to load source image from file '%hs'", path);
        return false;
    }

    if (!LoadBMP(file, path))
    {
        _fseeki64(file, SEEK_SET, 0);

        if (!LoadDDS(file, path))
        {
            RT_LOG_ERROR("Failed to load '%hs' - unknown format", path);
            fclose(file);
            return false;
        }
    }

    fclose(file);

    RT_LOG_ERROR("Bitmap '%hs' loaded: width=%u, height=%u", path, mWidth, mHeight);
    return true;
}

bool Bitmap::LoadBMP(FILE* file, const char* path)
{
    BITMAPFILEHEADER fileHeader;
    if (fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, file) != 1)
    {
        return false;
    }

    if (fileHeader.bfType != 0x4D42)
    {
        return false;
    }

    BITMAPINFOHEADER infoHeader;
    if (fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, file) != 1)
    {
        RT_LOG_ERROR("Failed to read BMP info header from file '%hs'", path);
        return false;
    }

    if (infoHeader.biPlanes != 1 || infoHeader.biCompression != BI_RGB || infoHeader.biBitCount != 24)
    {
        RT_LOG_ERROR("Unsupported BMP format: '%hs'", path);
        return false;
    }

    if (infoHeader.biWidth == 0 || infoHeader.biHeight == 0)
    {
        RT_LOG_ERROR("Invalid image size: '%hs'", path);
        return false;
    }

    if (!Init(infoHeader.biWidth, infoHeader.biHeight, Format::B8G8R8_Uint))
    {
        return false;
    }

    if (fread(mData, 3 * infoHeader.biWidth * infoHeader.biHeight, 1, file) != 1)
    {
        RT_LOG_ERROR("Failed to read bitmap data from file '%hs'", path);
        return false;
    }

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
        if (pf.dwFourCC == 114) // DXGI_FORMAT_R32_FLOAT
        {
            // format = Format::R32_Float;
        }
        if (pf.dwFourCC == 116) // D3DFMT_A32B32G32R32F
        {
            format = Format::R32G32B32A32_Float;
            linearSpace = true;
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

    const Uint32 dataSize = width * height * (Uint32)BitsPerPixel(format) / 8;
    if (fread(mData, dataSize, 1, file) != 1)
    {
        RT_LOG_ERROR("Failed to read bitmap data from file '%hs', errno = %u", path, errno);
        return false;
    }

    return true;
}

void Bitmap::Clear()
{
    if (mData)
    {
        const size_t dataSize = mWidth * mHeight * BitsPerPixel(mFormat) / 8;
        memset(mData, 0, dataSize);
    }
}

void Bitmap::SetPixel(Uint32 x, Uint32 y, const Vector4& value)
{
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
            Vector4* target = reinterpret_cast<Vector4*>(mData) + offset;
            *target = value;
            break;
        }
    }
}

Vector4 Bitmap::GetPixel(Uint32 x, Uint32 y) const
{
    using namespace math;

    const Uint32 offset = mWidth * y + x;

    switch (mFormat)
    {
        case Format::B8G8R8A8_Uint:
        {
            const Uint8* source = reinterpret_cast<Uint8*>(mData) + (4 * offset);
            return Vector4::Load4(source).Swizzle<2, 1, 0, 3>() / 255.0f;
        }

        case Format::R32G32B32_Float:
        {
            const float* source = reinterpret_cast<float*>(mData) + 3 * offset;
            return Vector4(source[0], source[1], source[2]);
        }

        case Format::R32G32B32A32_Float:
        {
            const Vector4* source = reinterpret_cast<Vector4*>(mData) + offset;
            return *source;
        }
    }

    return Vector4();
}

Vector4 Bitmap::Sample(Vector4 coords, const SamplerDesc& sampler) const
{
    // TODO
    (void)sampler;

    // TODO SSE this !!!!

    Int32 intU = (Int32)coords[0];
    Int32 intV = (Int32)coords[1];

    coords[0] -= (Float)intU;
    coords[1] -= (Float)intV;

    Int32 x = static_cast<Int32>(coords[0] * mWidth);
    Int32 y = static_cast<Int32>(coords[1] * mHeight);
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x >= mWidth) x = mWidth - 1;
    if (y >= mHeight) y = mHeight - 1;

    assert(x < mWidth);
    assert(y < mHeight);

    const Uint32 offset = mWidth * y + x;

    Vector4 color;
    switch (mFormat)
    {
        case Format::B8G8R8_Uint:
        {
            const Uint8* source = reinterpret_cast<Uint8*>(mData) + (3 * offset);
            const Vector4 raw = Vector4::Load4(source).Swizzle<2, 1, 0, 3>();
            color = (raw & VECTOR_MASK_XYZ) / 255.0f;
            break;
        }

        case Format::B8G8R8A8_Uint:
        {
            const Uint8* source = reinterpret_cast<Uint8*>(mData) + (4 * offset);
            color = Vector4::Load4(source).Swizzle<2, 1, 0, 3>() / 255.0f;
            break;
        }

        case Format::R32G32B32_Float:
        {
            const float* source = reinterpret_cast<float*>(mData) + 3 * offset;
            color = Vector4(source[0], source[1], source[2]);
            break;
        }

        case Format::R32G32B32A32_Float:
        {
            const Vector4* source = reinterpret_cast<Vector4*>(mData) + offset;
            color = *source;
            break;
        }
    }

    if (!mLinearSpace)
    {
        color *= color;
    }

    return color;
}

void Bitmap::WriteHorizontalLine(Uint32 y, const Vector4* values)
{
    const Uint32 offset = mWidth * y;

    switch (mFormat)
    {
        case Format::B8G8R8A8_Uint:
        {
            Uint8* target = reinterpret_cast<Uint8*>(mData) + 4 * offset;
            for (Uint32 x = 0; x < mWidth; ++x)
            {
                const Vector4 clampedValue = values[x].Swizzle<2, 1, 0, 3>() * 255.0f;
                clampedValue.Store4(target + 4 * x);
            }
            break;
        }

        case Format::R32G32B32_Float:
        {
            float* target = reinterpret_cast<float*>(mData) + 3 * offset;
            for (Uint32 x = 0; x < mWidth; ++x)
            {
                target[3 * x    ] = values[x][0];
                target[3 * x + 1] = values[x][1];
                target[3 * x + 2] = values[x][2];
            }
            break;
        }

        case Format::R32G32B32A32_Float:
        {
            Vector4* target = reinterpret_cast<Vector4*>(mData) + offset;
            for (Uint32 x = 0; x < mWidth; ++x)
            {
                target[x] = values[x];
            }
            break;
        }
    }
}

void Bitmap::WriteVerticalLine(Uint32 x, const Vector4* values)
{
    switch (mFormat)
    {
        case Format::B8G8R8A8_Uint:
        {
            Uint8* target = reinterpret_cast<Uint8*>(mData) + 4 * x;
            for (Uint32 y = 0; y < mHeight; ++y)
            {
                const Vector4 clampedValue = values[y].Swizzle<2, 1, 0, 3>() * 255.0f;
                clampedValue.Store4(target + 4 * mWidth * y);
            }
            break;
        }

        case Format::R32G32B32A32_Float:
        {
            Vector4* target = reinterpret_cast<Vector4*>(mData) + x;
            for (Uint32 y = 0; y < mHeight; ++y)
            {
                target[y * mWidth] = values[y];
            }
            break;
        }
    }
}

void Bitmap::ReadHorizontalLine(Uint32 y, Vector4* outValues) const
{
    const Uint32 offset = mWidth * y;

    switch (mFormat)
    {
        case Format::B8G8R8A8_Uint:
        {
            const Uint8* source = reinterpret_cast<Uint8*>(mData) + (4 * offset);
            for (Uint32 x = 0; x < mWidth; ++x)
            {
                outValues[x] = Vector4::Load4(source + 4 * x).Swizzle<2, 1, 0, 3>() / 255.0f;
            }
            break;
        }

        case Format::R32G32B32_Float:
        {
            const float* source = reinterpret_cast<float*>(mData) + 3 * offset;
            for (Uint32 x = 0; x < mWidth; ++x)
            {
                outValues[x][0] = source[3 * x    ];
                outValues[x][1] = source[3 * x + 1];
                outValues[x][2] = source[3 * x + 2];
                outValues[x][3] = 0.0f;
            }
            break;
        }

        case Format::R32G32B32A32_Float:
        {
            const Vector4* source = reinterpret_cast<Vector4*>(mData) + offset;
            for (Uint32 x = 0; x < mWidth; ++x)
            {
                outValues[x] = source[x];
            }
            break;
        }
    }
}

void Bitmap::ReadVerticalLine(Uint32 x, Vector4* outValues) const
{
    switch (mFormat)
    {
        case Format::B8G8R8A8_Uint:
        {
            const Uint8* source = reinterpret_cast<Uint8*>(mData) + 4 * x;
            for (Uint32 y = 0; y < mHeight; ++y)
            {
                outValues[x] = Vector4::Load4(source + 4 * mWidth * y).Swizzle<2, 1, 0, 3>() / 255.0f;
            }
            break;
        }

        case Format::R32G32B32A32_Float:
        {
            const Vector4* source = reinterpret_cast<Vector4*>(mData) + x;
            for (Uint32 y = 0; y < mHeight; ++y)
            {
                outValues[y] = source[y * mWidth];
            }
            break;
        }
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

void Bitmap::BoxBlur_Internal(Vector4* targetLine, const Vector4* srcLine,
                              const Uint32 radius, const Uint32 width, const Float factor)
{
    Uint32 ti = 0;
    Uint32 li = 0;
    Uint32 ri = radius;

    Vector4 fv = srcLine[0];
    Vector4 lv = srcLine[width - 1];
    Vector4 val = fv * (Float)(radius + 1);

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

    Vector4 srcLine[MaxSize];
    Vector4 targetLine[MaxSize];

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

    Vector4 srcLine[MaxSize];
    Vector4 targetLine[MaxSize];

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
