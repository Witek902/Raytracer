#include "PCH.h"
#include "Bitmap.h"
#include "Logger.h"

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

namespace rt {

using namespace math;

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

} // namespace rt
