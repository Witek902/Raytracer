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
        Uint32 size;
        Uint32 flags;
        Uint32 fourCC;
        Uint32 rgbBitCount;
        Uint32 rBitMask;
        Uint32 gBitMask;
        Uint32 bBitMask;
        Uint32 aBitMask;
    };

    struct Header
    {
        Uint32 magic;
        Uint32 size;
        Uint32 flags;
        Uint32 height;
        Uint32 width;
        Uint32 pitchOrLinearSize;
        Uint32 depth;
        Uint32 mipMapCount;
        Uint32 reserved1[11];

        //  DDPIXELFORMAT
        DDS_PIXELFORMAT pixelFormat;

        //  DDCAPS2
        struct
        {
            Uint32 caps1;
            Uint32 caps2;
            Uint32 DDSX;
            Uint32 reserved;
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
    if (header.magic != DDS_MAGIC_NUMBER)
    {
        return false;
    }

    Uint32 width = header.width;
    Uint32 height = header.height;
    if (width < 1 || height < 1 || width >= std::numeric_limits<Uint16>::max() || height >= std::numeric_limits<Uint16>::max())
    {
        RT_LOG_ERROR("Unsupported DDS format in file '%hs'", path);
        return false;
    }

    bool linearSpace = false;
    Format format = Format::Unknown;
    const DDS_PIXELFORMAT& pf = header.pixelFormat;
    if (pf.flags & DDPF_RGB)
    {
        if (pf.rgbBitCount == 32)
        {
            if (pf.rBitMask == 0xFF && pf.gBitMask == 0xFF && pf.bBitMask == 0xFF && pf.aBitMask == 0xFF)
            {
                format = Format::B8G8R8A8_UNorm; // DXGI_FORMAT_R8G8B8A8_UNORM;
            }
            else if (pf.rBitMask == 0xFFFFFFFF && pf.gBitMask == 0 && pf.bBitMask == 0 && pf.aBitMask == 0)
            {
                format = Format::R32_Float; // DXGI_FORMAT_R32_FLOAT;
            }
            if (pf.rBitMask == 0xFFFF && pf.gBitMask == 0xFFFF0000 && pf.bBitMask == 0 && pf.aBitMask == 0)
            {
                format = Format::R16G16_UNorm; // DXGI_FORMAT_R16G16_UNORM;
            }
        }
    }
    else if (pf.flags & DDPF_FOURCC)
    {
        if (pf.fourCC == 111) // D3DFMT_R16F
        {
            format = Format::R16_Half;
        }
        else if (pf.fourCC == 112) // D3DFMT_G16R16F
        {
            format = Format::R16G16_Half;
        }
        else if (pf.fourCC == 113) // D3DFMT_A16B16G16R16F
        {
            format = Format::R16G16B16A16_Half;
        }
        else if (pf.fourCC == 114) // DXGI_FORMAT_R32_FLOAT
        {
            format = Format::R32_Float;
        }
        else if (pf.fourCC == 116) // D3DFMT_A32B32G32R32F
        {
            format = Format::R32G32B32A32_Float;
            linearSpace = true;
        }
        else if (pf.fourCC == 36) // D3DFMT_A16B16G16R16
        {
            format = Format::R16G16B16A16_UNorm;
        }
        else if (MAKEFOURCC('D', 'X', 'T', '1') == pf.fourCC)
            format = Format::BC1; //DXGI_FORMAT_BC1_UNORM
        else if (MAKEFOURCC('A', 'T', 'I', '1') == pf.fourCC)
            format = Format::BC4; //DXGI_FORMAT_BC4_UNORM
        else if (MAKEFOURCC('B', 'C', '4', 'U') == pf.fourCC)
            format = Format::BC4; //DXGI_FORMAT_BC4_UNORM
        else if (MAKEFOURCC('B', 'C', '4', 'S') == pf.fourCC)
            format = Format::BC4; //DXGI_FORMAT_BC4_SNORM
        else if (MAKEFOURCC('A', 'T', 'I', '2') == pf.fourCC)
            format = Format::BC5; //DXGI_FORMAT_BC5_UNORM
        else if (MAKEFOURCC('B', 'C', '5', 'U') == pf.fourCC)
            format = Format::BC5; //DXGI_FORMAT_BC5_UNORM
        else if (MAKEFOURCC('D', 'X', '1', '0') == pf.fourCC)
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
                format = Format::B8G8R8A8_UNorm;
                linearSpace = true;
            }
        }
    }
    else if (pf.flags & DDPF_LUMINANCE)
    {
        if (pf.rgbBitCount == 8)
        {
            if (pf.rBitMask == 0xFF && pf.gBitMask == 0x0 && pf.bBitMask == 0x0 && pf.aBitMask == 0x0)
            {
                format = Format::R8_UNorm; // D3DX10/11 writes this out as DX10 extension
            }
            else if (pf.rBitMask == 0xFF && pf.gBitMask == 0x0 && pf.bBitMask == 0x0 && pf.aBitMask == 0xFF00)
            {
                format = Format::R8G8_UNorm; // Some DDS writers assume the bitcount should be 8 instead of 16
            }
        }
        else if (pf.rgbBitCount == 16)
        {
            if (pf.rBitMask == 0xFFFF && pf.gBitMask == 0x0 && pf.bBitMask == 0x0 && pf.aBitMask == 0x0)
            {
                format = Format::R16_UNorm; // D3DX10/11 writes this out as DX10 extension
            }
            else if (pf.rBitMask == 0xFF && pf.gBitMask == 0 && pf.bBitMask == 0 && pf.aBitMask == 0xFF00)
            {
                format = Format::R8G8_UNorm; // Some DDS writers assume the bitcount should be 8 instead of 16
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
