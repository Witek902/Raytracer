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

typedef enum _D3DFORMAT
{
    D3DFMT_UNKNOWN              = 0,
    D3DFMT_R8G8B8               = 20,
    D3DFMT_A8R8G8B8             = 21,
    D3DFMT_X8R8G8B8             = 22,
    D3DFMT_R5G6B5               = 23,
    D3DFMT_X1R5G5B5             = 24,
    D3DFMT_A1R5G5B5             = 25,
    D3DFMT_A4R4G4B4             = 26,
    D3DFMT_R3G3B2               = 27,
    D3DFMT_A8                   = 28,
    D3DFMT_A8R3G3B2             = 29,
    D3DFMT_X4R4G4B4             = 30,
    D3DFMT_A2B10G10R10          = 31,
    D3DFMT_A8B8G8R8             = 32,
    D3DFMT_X8B8G8R8             = 33,
    D3DFMT_G16R16               = 34,
    D3DFMT_A2R10G10B10          = 35,
    D3DFMT_A16B16G16R16         = 36,
    D3DFMT_A8P8                 = 40,
    D3DFMT_P8                   = 41,
    D3DFMT_L8                   = 50,
    D3DFMT_A8L8                 = 51,
    D3DFMT_A4L4                 = 52,
    D3DFMT_V8U8                 = 60,
    D3DFMT_L6V5U5               = 61,
    D3DFMT_X8L8V8U8             = 62,
    D3DFMT_Q8W8V8U8             = 63,
    D3DFMT_V16U16               = 64,
    D3DFMT_A2W10V10U10          = 67,
    D3DFMT_UYVY                 = MAKEFOURCC('U', 'Y', 'V', 'Y'),
    D3DFMT_R8G8_B8G8            = MAKEFOURCC('R', 'G', 'B', 'G'),
    D3DFMT_YUY2                 = MAKEFOURCC('Y', 'U', 'Y', '2'),
    D3DFMT_G8R8_G8B8            = MAKEFOURCC('G', 'R', 'G', 'B'),
    D3DFMT_DXT1                 = MAKEFOURCC('D', 'X', 'T', '1'),
    D3DFMT_DXT2                 = MAKEFOURCC('D', 'X', 'T', '2'),
    D3DFMT_DXT3                 = MAKEFOURCC('D', 'X', 'T', '3'),
    D3DFMT_DXT4                 = MAKEFOURCC('D', 'X', 'T', '4'),
    D3DFMT_DXT5                 = MAKEFOURCC('D', 'X', 'T', '5'),
    D3DFMT_D16_LOCKABLE         = 70,
    D3DFMT_D32                  = 71,
    D3DFMT_D15S1                = 73,
    D3DFMT_D24S8                = 75,
    D3DFMT_D24X8                = 77,
    D3DFMT_D24X4S4              = 79,
    D3DFMT_D16                  = 80,
    D3DFMT_D32F_LOCKABLE        = 82,
    D3DFMT_D24FS8               = 83,
    D3DFMT_D32_LOCKABLE         = 84,
    D3DFMT_S8_LOCKABLE          = 85,
    D3DFMT_L16                  = 81,
    D3DFMT_VERTEXDATA           = 100,
    D3DFMT_INDEX16              = 101,
    D3DFMT_INDEX32              = 102,
    D3DFMT_Q16W16V16U16         = 110,
    D3DFMT_MULTI2_ARGB8         = MAKEFOURCC('M','E','T','1'),
    D3DFMT_R16F                 = 111,
    D3DFMT_G16R16F              = 112,
    D3DFMT_A16B16G16R16F        = 113,
    D3DFMT_R32F                 = 114,
    D3DFMT_G32R32F              = 115,
    D3DFMT_A32B32G32R32F        = 116,
    D3DFMT_CxV8U8               = 117,
    D3DFMT_A1                   = 118,
    D3DFMT_A2B10G10R10_XR_BIAS  = 119,
    D3DFMT_BINARYBUFFER         = 199,
    D3DFMT_FORCE_DWORD          = 0x7fffffff,
} D3DFORMAT;

typedef enum DXGI_FORMAT
{
    DXGI_FORMAT_UNKNOWN	                    = 0,
    DXGI_FORMAT_R32G32B32A32_TYPELESS       = 1,
    DXGI_FORMAT_R32G32B32A32_FLOAT          = 2,
    DXGI_FORMAT_R32G32B32A32_UINT           = 3,
    DXGI_FORMAT_R32G32B32A32_SINT           = 4,
    DXGI_FORMAT_R32G32B32_TYPELESS          = 5,
    DXGI_FORMAT_R32G32B32_FLOAT             = 6,
    DXGI_FORMAT_R32G32B32_UINT              = 7,
    DXGI_FORMAT_R32G32B32_SINT              = 8,
    DXGI_FORMAT_R16G16B16A16_TYPELESS       = 9,
    DXGI_FORMAT_R16G16B16A16_FLOAT          = 10,
    DXGI_FORMAT_R16G16B16A16_UNORM          = 11,
    DXGI_FORMAT_R16G16B16A16_UINT           = 12,
    DXGI_FORMAT_R16G16B16A16_SNORM          = 13,
    DXGI_FORMAT_R16G16B16A16_SINT           = 14,
    DXGI_FORMAT_R32G32_TYPELESS             = 15,
    DXGI_FORMAT_R32G32_FLOAT                = 16,
    DXGI_FORMAT_R32G32_UINT                 = 17,
    DXGI_FORMAT_R32G32_SINT                 = 18,
    DXGI_FORMAT_R32G8X24_TYPELESS           = 19,
    DXGI_FORMAT_D32_FLOAT_S8X24_UINT        = 20,
    DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS    = 21,
    DXGI_FORMAT_X32_TYPELESS_G8X24_UINT     = 22,
    DXGI_FORMAT_R10G10B10A2_TYPELESS        = 23,
    DXGI_FORMAT_R10G10B10A2_UNORM           = 24,
    DXGI_FORMAT_R10G10B10A2_UINT            = 25,
    DXGI_FORMAT_R11G11B10_FLOAT             = 26,
    DXGI_FORMAT_R8G8B8A8_TYPELESS           = 27,
    DXGI_FORMAT_R8G8B8A8_UNORM              = 28,
    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB         = 29,
    DXGI_FORMAT_R8G8B8A8_UINT               = 30,
    DXGI_FORMAT_R8G8B8A8_SNORM              = 31,
    DXGI_FORMAT_R8G8B8A8_SINT               = 32,
    DXGI_FORMAT_R16G16_TYPELESS             = 33,
    DXGI_FORMAT_R16G16_FLOAT                = 34,
    DXGI_FORMAT_R16G16_UNORM                = 35,
    DXGI_FORMAT_R16G16_UINT                 = 36,
    DXGI_FORMAT_R16G16_SNORM                = 37,
    DXGI_FORMAT_R16G16_SINT                 = 38,
    DXGI_FORMAT_R32_TYPELESS                = 39,
    DXGI_FORMAT_D32_FLOAT                   = 40,
    DXGI_FORMAT_R32_FLOAT                   = 41,
    DXGI_FORMAT_R32_UINT                    = 42,
    DXGI_FORMAT_R32_SINT                    = 43,
    DXGI_FORMAT_R24G8_TYPELESS              = 44,
    DXGI_FORMAT_D24_UNORM_S8_UINT           = 45,
    DXGI_FORMAT_R24_UNORM_X8_TYPELESS       = 46,
    DXGI_FORMAT_X24_TYPELESS_G8_UINT        = 47,
    DXGI_FORMAT_R8G8_TYPELESS               = 48,
    DXGI_FORMAT_R8G8_UNORM                  = 49,
    DXGI_FORMAT_R8G8_UINT                   = 50,
    DXGI_FORMAT_R8G8_SNORM                  = 51,
    DXGI_FORMAT_R8G8_SINT                   = 52,
    DXGI_FORMAT_R16_TYPELESS                = 53,
    DXGI_FORMAT_R16_FLOAT                   = 54,
    DXGI_FORMAT_D16_UNORM                   = 55,
    DXGI_FORMAT_R16_UNORM                   = 56,
    DXGI_FORMAT_R16_UINT                    = 57,
    DXGI_FORMAT_R16_SNORM                   = 58,
    DXGI_FORMAT_R16_SINT                    = 59,
    DXGI_FORMAT_R8_TYPELESS                 = 60,
    DXGI_FORMAT_R8_UNORM                    = 61,
    DXGI_FORMAT_R8_UINT                     = 62,
    DXGI_FORMAT_R8_SNORM                    = 63,
    DXGI_FORMAT_R8_SINT                     = 64,
    DXGI_FORMAT_A8_UNORM                    = 65,
    DXGI_FORMAT_R1_UNORM                    = 66,
    DXGI_FORMAT_R9G9B9E5_SHAREDEXP          = 67,
    DXGI_FORMAT_R8G8_B8G8_UNORM             = 68,
    DXGI_FORMAT_G8R8_G8B8_UNORM             = 69,
    DXGI_FORMAT_BC1_TYPELESS                = 70,
    DXGI_FORMAT_BC1_UNORM                   = 71,
    DXGI_FORMAT_BC1_UNORM_SRGB              = 72,
    DXGI_FORMAT_BC2_TYPELESS                = 73,
    DXGI_FORMAT_BC2_UNORM                   = 74,
    DXGI_FORMAT_BC2_UNORM_SRGB              = 75,
    DXGI_FORMAT_BC3_TYPELESS                = 76,
    DXGI_FORMAT_BC3_UNORM                   = 77,
    DXGI_FORMAT_BC3_UNORM_SRGB              = 78,
    DXGI_FORMAT_BC4_TYPELESS                = 79,
    DXGI_FORMAT_BC4_UNORM                   = 80,
    DXGI_FORMAT_BC4_SNORM                   = 81,
    DXGI_FORMAT_BC5_TYPELESS                = 82,
    DXGI_FORMAT_BC5_UNORM                   = 83,
    DXGI_FORMAT_BC5_SNORM                   = 84,
    DXGI_FORMAT_B5G6R5_UNORM                = 85,
    DXGI_FORMAT_B5G5R5A1_UNORM              = 86,
    DXGI_FORMAT_B8G8R8A8_UNORM              = 87,
    DXGI_FORMAT_B8G8R8X8_UNORM              = 88,
    DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM  = 89,
    DXGI_FORMAT_B8G8R8A8_TYPELESS           = 90,
    DXGI_FORMAT_B8G8R8A8_UNORM_SRGB         = 91,
    DXGI_FORMAT_B8G8R8X8_TYPELESS           = 92,
    DXGI_FORMAT_B8G8R8X8_UNORM_SRGB         = 93,
    DXGI_FORMAT_BC6H_TYPELESS               = 94,
    DXGI_FORMAT_BC6H_UF16                   = 95,
    DXGI_FORMAT_BC6H_SF16                   = 96,
    DXGI_FORMAT_BC7_TYPELESS                = 97,
    DXGI_FORMAT_BC7_UNORM                   = 98,
    DXGI_FORMAT_BC7_UNORM_SRGB              = 99,
    DXGI_FORMAT_AYUV                        = 100,
    DXGI_FORMAT_Y410                        = 101,
    DXGI_FORMAT_Y416                        = 102,
    DXGI_FORMAT_NV12                        = 103,
    DXGI_FORMAT_P010                        = 104,
    DXGI_FORMAT_P016                        = 105,
    DXGI_FORMAT_420_OPAQUE                  = 106,
    DXGI_FORMAT_YUY2                        = 107,
    DXGI_FORMAT_Y210                        = 108,
    DXGI_FORMAT_Y216                        = 109,
    DXGI_FORMAT_NV11                        = 110,
    DXGI_FORMAT_AI44                        = 111,
    DXGI_FORMAT_IA44                        = 112,
    DXGI_FORMAT_P8                          = 113,
    DXGI_FORMAT_A8P8                        = 114,
    DXGI_FORMAT_B4G4R4A4_UNORM              = 115,
    DXGI_FORMAT_P208                        = 130,
    DXGI_FORMAT_V208                        = 131,
    DXGI_FORMAT_V408                        = 132,
    DXGI_FORMAT_FORCE_UINT                  = 0xffffffff,
} DXGI_FORMAT;

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

    InitData initData;

    initData.linearSpace = true;
    initData.width = header.width;
    initData.height = header.height;
    if (initData.width < 1 || initData.height < 1 || initData.width >= std::numeric_limits<Uint16>::max() || initData.height >= std::numeric_limits<Uint16>::max())
    {
        RT_LOG_ERROR("Unsupported DDS format in file '%hs': dimensions are out of bounds (%ux%u)", path, initData.width, initData.height);
        return false;
    }

    const DDS_PIXELFORMAT& pf = header.pixelFormat;
    if (pf.flags & DDPF_RGB)
    {
        if (pf.rgbBitCount == 32)
        {
            if (pf.rBitMask == 0x00FF0000 && pf.gBitMask == 0x0000FF00 && pf.bBitMask == 0x000000FF && pf.aBitMask == 0xFF000000)
            {
                initData.format = Format::B8G8R8A8_UNorm;
                initData.linearSpace = false;
            }
            else if (pf.rBitMask == 0x000000FF && pf.gBitMask == 0x0000FF00 && pf.bBitMask == 0x00FF0000 && pf.aBitMask == 0xFF000000)
            {
                initData.format = Format::R8G8B8A8_UNorm;
                initData.linearSpace = false;
            }
            else if (pf.rBitMask == 0xFFFFFFFF && pf.gBitMask == 0 && pf.bBitMask == 0 && pf.aBitMask == 0)
            {
                initData.format = Format::R32_Float;
            }
            else if (pf.rBitMask == 0xFFFF && pf.gBitMask == 0xFFFF0000 && pf.bBitMask == 0 && pf.aBitMask == 0)
            {
                initData.format = Format::R16G16_UNorm;
            }
        }
        else if (pf.rgbBitCount == 16)
        {
            if (pf.rBitMask == 0xf800 && pf.gBitMask == 0x07e0 && pf.bBitMask == 0x001f && pf.aBitMask == 0x0000)
            {
                initData.format = Format::B5G6R5_UNorm;
                initData.linearSpace = false;
            }
        }
    }
    else if (pf.flags & DDPF_FOURCC)
    {
        if (pf.fourCC == D3DFMT_R16F)
        {
            initData.format = Format::R16_Half;
        }
        else if (pf.fourCC == D3DFMT_G16R16F)
        {
            initData.format = Format::R16G16_Half;
        }
        else if (pf.fourCC == D3DFMT_A16B16G16R16F)
        {
            initData.format = Format::R16G16B16A16_Half;
        }
        else if (pf.fourCC == D3DFMT_R32F)
        {
            initData.format = Format::R32_Float;
        }
        else if (pf.fourCC == D3DFMT_G32R32F)
        {
            initData.format = Format::R32G32_Float;
        }
        else if (pf.fourCC == D3DFMT_A32B32G32R32F)
        {
            initData.format = Format::R32G32B32A32_Float;
        }
        else if (pf.fourCC == D3DFMT_A16B16G16R16)
        {
            initData.format = Format::R16G16B16A16_UNorm;
        }
        else if (pf.fourCC == D3DFMT_DXT1)
        {
            initData.format = Format::BC1;
        }
        else if (MAKEFOURCC('A', 'T', 'I', '1') == pf.fourCC)
        {
            initData.format = Format::BC4; //DXGI_FORMAT_BC4_UNORM
        }
        else if (MAKEFOURCC('B', 'C', '4', 'U') == pf.fourCC)
        {
            initData.format = Format::BC4; //DXGI_FORMAT_BC4_UNORM
        }
        else if (MAKEFOURCC('B', 'C', '4', 'S') == pf.fourCC)
        {
            initData.format = Format::BC4; //DXGI_FORMAT_BC4_SNORM
        }
        else if (MAKEFOURCC('A', 'T', 'I', '2') == pf.fourCC)
        {
            initData.format = Format::BC5; //DXGI_FORMAT_BC5_UNORM
        }
        else if (MAKEFOURCC('B', 'C', '5', 'U') == pf.fourCC)
        {
            initData.format = Format::BC5; //DXGI_FORMAT_BC5_UNORM
        }
        else if (MAKEFOURCC('D', 'X', '1', '0') == pf.fourCC)
        {
            // read DX10 header
            HeaderDX10 headerDX10;
            if (fread(&headerDX10, sizeof(headerDX10), 1, file) != 1)
            {
                RT_LOG_ERROR("Failed to read DX10 header '%hs'", path);
                return false;
            }

            if (headerDX10.dxgiFormat == DXGI_FORMAT_R16G16B16A16_FLOAT)
            {
                initData.format = Format::R16G16B16A16_Half;
            }
            else if (headerDX10.dxgiFormat == DXGI_FORMAT_R16G16_FLOAT)
            {
                initData.format = Format::R16G16_Half;
            }
            else if (headerDX10.dxgiFormat == DXGI_FORMAT_R16_FLOAT)
            {
                initData.format = Format::R16_Half;
            }
            if (headerDX10.dxgiFormat == DXGI_FORMAT_R32G32B32A32_FLOAT)
            {
                initData.format = Format::R32G32B32A32_Float;
            }
            else if (headerDX10.dxgiFormat == DXGI_FORMAT_R32G32B32_FLOAT)
            {
                initData.format = Format::R32G32B32_Float;
            }
            else if (headerDX10.dxgiFormat == DXGI_FORMAT_R32G32_FLOAT)
            {
                initData.format = Format::R32G32_Float;
            }
            else if (headerDX10.dxgiFormat == DXGI_FORMAT_R32_FLOAT)
            {
                initData.format = Format::R32_Float;
            }
            else if (headerDX10.dxgiFormat == DXGI_FORMAT_R9G9B9E5_SHAREDEXP)
            {
                initData.format = Format::R9G9B9E5_SharedExp;
                initData.linearSpace = true;
            }
            else if (headerDX10.dxgiFormat == DXGI_FORMAT_R11G11B10_FLOAT)
            {
                initData.format = Format::R11G11B10_Float;
                initData.linearSpace = true;
            }
            else if (headerDX10.dxgiFormat == DXGI_FORMAT_B8G8R8A8_UNORM)
            {
                initData.format = Format::B8G8R8A8_UNorm;
            }
            else if (headerDX10.dxgiFormat == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB)
            {
                initData.format = Format::B8G8R8A8_UNorm;
                initData.linearSpace = false;
            }
            else if (headerDX10.dxgiFormat == DXGI_FORMAT_R8G8_UNORM)
            {
                initData.format = Format::R8G8_UNorm;
            }
            else if (headerDX10.dxgiFormat == DXGI_FORMAT_R8_UNORM)
            {
                initData.format = Format::R8_UNorm;
            }
            else if (headerDX10.dxgiFormat == DXGI_FORMAT_B5G6R5_UNORM)
            {
                initData.format = Format::B5G6R5_UNorm;
                initData.linearSpace = false;
            }
            else if (headerDX10.dxgiFormat == DXGI_FORMAT_R16G16B16A16_UNORM)
            {
                initData.format = Format::R16G16B16A16_UNorm;
            }
            else if (headerDX10.dxgiFormat == DXGI_FORMAT_R16G16_UNORM)
            {
                initData.format = Format::R16G16_UNorm;
            }
            else if (headerDX10.dxgiFormat == DXGI_FORMAT_R16_UNORM)
            {
                initData.format = Format::R16_UNorm;
            }
            else if (headerDX10.dxgiFormat == DXGI_FORMAT_BC1_UNORM)
            {
                initData.format = Format::BC1;
            }
            else if (headerDX10.dxgiFormat == DXGI_FORMAT_BC1_UNORM_SRGB)
            {
                initData.format = Format::BC1;
                initData.linearSpace = false;
            }
            else if (headerDX10.dxgiFormat == DXGI_FORMAT_BC4_UNORM)
            {
                initData.format = Format::BC4;
            }
            else if (headerDX10.dxgiFormat == DXGI_FORMAT_BC5_UNORM)
            {
                initData.format = Format::BC5;
            }
        }
    }
    else if (pf.flags & DDPF_LUMINANCE)
    {
        if (pf.rgbBitCount == 8)
        {
            if (pf.rBitMask == 0xFF && pf.gBitMask == 0x0 && pf.bBitMask == 0x0 && pf.aBitMask == 0x0)
            {
                initData.format = Format::R8_UNorm; // D3DX10/11 writes this out as DX10 extension
                initData.linearSpace = false;
            }
            else if (pf.rBitMask == 0xFF && pf.gBitMask == 0x0 && pf.bBitMask == 0x0 && pf.aBitMask == 0xFF00)
            {
                initData.format = Format::R8G8_UNorm; // Some DDS writers assume the bitcount should be 8 instead of 16
                initData.linearSpace = false;
            }
        }
        else if (pf.rgbBitCount == 16)
        {
            if (pf.rBitMask == 0xFFFF && pf.gBitMask == 0x0 && pf.bBitMask == 0x0 && pf.aBitMask == 0x0)
            {
                initData.format = Format::R16_UNorm; // D3DX10/11 writes this out as DX10 extension
            }
            else if (pf.rBitMask == 0xFF && pf.gBitMask == 0 && pf.bBitMask == 0 && pf.aBitMask == 0xFF00)
            {
                initData.format = Format::R8G8_UNorm; // Some DDS writers assume the bitcount should be 8 instead of 16
                initData.linearSpace = false;
            }
        }
    }

    if (initData.format == Format::Unknown)
    {
        RT_LOG_ERROR("Unsupported DDS format in file '%hs'", path);
        return false;
    }

    if (!Init(initData))
    {
        return false;
    }

    // TODO support for DXT textures that has dimension non-4-multiply
    const size_t dataSize = ComputeDataSize(initData);
    if (fread(mData, dataSize, 1, file) != 1)
    {
        RT_LOG_ERROR("Failed to read bitmap data from file '%hs', errno = %u", path, errno);
        return false;
    }

    return true;
}

} // namespace rt
