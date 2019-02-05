#include "PCH.h"
#include "Bitmap.h"
#include "Logger.h"

namespace rt {

using namespace math;

namespace
{
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

    const Uint32 lineSize = infoHeader.biWidth * BitsPerPixel(mFormat) / 8;

    // Note: one line size in BMP file must be multiple of 4 bytes
    const Uint32 linePadding = math::RoundUp(lineSize, 4u) - lineSize;

    for (Uint32 i = 0; i < infoHeader.biHeight; ++i)
    {
        if (fread(mData + lineSize * i, lineSize, 1, file) != 1)
        {
            RT_LOG_ERROR("Failed to read bitmap data from file '%hs'", path);
            return false;
        }

        if (fseek(file, linePadding, SEEK_CUR) != 0)
        {
            RT_LOG_ERROR("Failed to read bitmap data from file '%hs'", path);
            return false;
        }
    }

    return true;
}

bool Bitmap::SaveBMP(const char* path, bool flipVertically) const
{
    Uint32 dataSize = 3 * mWidth * mHeight;

    std::vector<Uint8> tmpData(dataSize);
    const Uint8* dataPtr = nullptr;

    if (mFormat == Format::B8G8R8A8_Uint)
    {
        const Uint8* rawData = reinterpret_cast<const Uint8*>(mData);

        Uint32 i = 0;
        for (Uint32 y = 0; y < (Uint32)mHeight; ++y)
        {
            const Uint32 realY = flipVertically ? mHeight - 1 - y : y;
            for (Uint32 x = 0; x < (Uint32)mWidth; ++x)
            {
                const Uint32 p = mWidth * realY + x;
                tmpData[i++] = rawData[4 * p];
                tmpData[i++] = rawData[4 * p + 1];
                tmpData[i++] = rawData[4 * p + 2];
            }
        }

        dataPtr = tmpData.data();
    }
    else if (mFormat == Format::B8G8R8_Uint)
    {
        dataPtr = reinterpret_cast<const Uint8*>(mData);
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
