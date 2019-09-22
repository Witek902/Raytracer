#include "PCH.h"
#include "Bitmap.h"
#include "Logger.h"
#include "../Containers/DynArray.h"

namespace rt {

using namespace math;

namespace
{
#pragma pack(push, 2)

    struct BitmapFileHeader
    {
        uint16 bfType;
        uint32 bfSize;
        uint16 bfReserved1;
        uint16 bfReserved2;
        uint32 bfOffBits;
    };

    struct BitmapInfoHeader
    {
        uint32 biSize;
        uint32 biWidth;
        uint32 biHeight;
        uint16 biPlanes;
        uint16 biBitCount;
        uint32 biCompression;
        uint32 biSizeImage;
        uint32 biXPelsPerMeter;
        uint32 biYPelsPerMeter;
        uint32 biClrUsed;
        uint32 biClrImportant;
    };

    struct BMPHeader
    {
        BitmapFileHeader fileHeader;
        BitmapInfoHeader infoHeader;
    };

#pragma pack(pop)
} // namespace

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

    InitData initData;

    if (infoHeader.biBitCount == 24)
    {
        initData.format = Format::B8G8R8_UNorm;
    }
    else if (infoHeader.biBitCount == 8 && infoHeader.biClrUsed > 0)
    {
        initData.format = Format::B8G8R8A8_UNorm_Palette;
    }
    else if (infoHeader.biBitCount == 8 && infoHeader.biClrUsed == 0)
    {
        initData.format = Format::R8_UNorm;
    }
    else
    {
        RT_LOG_ERROR("Unsupported BMP bit depth (%u): '%hs'", (uint32)infoHeader.biBitCount, path);
        return false;
    }

    if (infoHeader.biWidth == 0 || infoHeader.biHeight == 0)
    {
        RT_LOG_ERROR("Invalid image size: '%hs'", path);
        return false;
    }

    initData.linearSpace = false;
    initData.width = infoHeader.biWidth;
    initData.height = infoHeader.biHeight;
    // Note: one line size in BMP file must be multiple of 4 bytes
    initData.stride = math::RoundUp(infoHeader.biWidth * BitsPerPixel(initData.format) / 8, 4u);
    initData.paletteSize = infoHeader.biClrUsed;

    if (!Init(initData))
    {
        return false;
    }

    const size_t paletteBytes = (size_t)infoHeader.biClrUsed * 4u;
    if (paletteBytes > 0)
    {
        if (fread(mPalette, paletteBytes, 1, file) != 1)
        {
            RT_LOG_ERROR("Failed to read bitmap palette from file '%hs'", path);
            return false;
        }
    }

    if (fseek(file, fileHeader.bfOffBits, SEEK_SET) != 0)
    {
        RT_LOG_ERROR("Failed to read bitmap data from file '%hs'", path);
        return false;
    }

    if (fread(mData, size_t(initData.stride) * size_t(initData.height), 1, file) != 1)
    {
        RT_LOG_ERROR("Failed to read bitmap data from file '%hs'", path);
        return false;
    }

    return true;
}

bool Bitmap::SaveBMP(const char* path, bool flipVertically) const
{
    uint32 dataSize = 3 * mWidth * mHeight;

    DynArray<uint8> tmpData(dataSize);
    const uint8* dataPtr = nullptr;

    if (mFormat == Format::B8G8R8A8_UNorm)
    {
        const uint8* rawData = reinterpret_cast<const uint8*>(mData);

        uint32 i = 0;
        for (uint32 y = 0; y < (uint32)mHeight; ++y)
        {
            const uint32 realY = flipVertically ? mHeight - 1 - y : y;
            for (uint32 x = 0; x < (uint32)mWidth; ++x)
            {
                const uint32 p = mWidth * realY + x;
                tmpData[i++] = rawData[4 * p];
                tmpData[i++] = rawData[4 * p + 1];
                tmpData[i++] = rawData[4 * p + 2];
            }
        }

        dataPtr = tmpData.Data();
    }
    else if (mFormat == Format::B8G8R8_UNorm)
    {
        dataPtr = reinterpret_cast<const uint8*>(mData);
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
            /* bfSize */        static_cast<uint32>(sizeof(BMPHeader) + dataSize),
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

    if (fwrite(dataPtr, dataSize, 1, file) != 1)
    {
        RT_LOG_ERROR("Failed to write bitmap image data to file '%s', code = %u", path, stderr);
        fclose(file);
        return false;
    }

    RT_LOG_INFO("Image file '%s' written successfully", path);
    fclose(file);
    return true;
}

} // namespace rt
