#include "PCH.h"
#include "Bitmap.h"
#include "Logger.h"
#include "../Containers/DynArray.h"
#include "../External/tinyexr/tinyexr.h"

namespace rt {

using namespace math;

bool Bitmap::LoadEXR(FILE* file, const char* path)
{
    RT_UNUSED(file);

    // 1. Read EXR version.
    EXRVersion exrVersion;

    int ret = ParseEXRVersionFromFile(&exrVersion, path);
    if (ret != 0)
    {
        RT_LOG_ERROR("Invalid EXR file: %s", path);
        return false;
    }

    if (exrVersion.multipart)
    {
        RT_LOG_ERROR("Multipart EXR are not supported: %s", path);
        return false;
    }

    // 2. Read EXR header
    EXRHeader exrHeader;
    InitEXRHeader(&exrHeader);

    const char* err = NULL;
    ret = ParseEXRHeaderFromFile(&exrHeader, &exrVersion, path, &err);
    if (ret != 0)
    {
        RT_LOG_ERROR("Parse EXR error: %s", err);
        FreeEXRErrorMessage(err);
        return ret;
    }

    EXRImage exrImage;
    InitEXRImage(&exrImage);

    ret = LoadEXRImageFromFile(&exrImage, &exrHeader, path, &err);
    if (ret != 0)
    {
        RT_LOG_ERROR("Load EXR error: %s", err);
        FreeEXRErrorMessage(err);
        return ret;
    }

    if (!exrImage.images)
    {
        RT_LOG_ERROR("Tiled EXR are not supported: %s", path);
        FreeEXRImage(&exrImage);
        return false;
    }

    InitData initData;
    initData.linearSpace = true;
    initData.width = exrImage.width;
    initData.height = exrImage.height;

    if (exrHeader.num_channels == 3)
    {
        const bool sameFormat = exrHeader.pixel_types[0] == exrHeader.pixel_types[1] && exrHeader.pixel_types[0] == exrHeader.pixel_types[2];
        if (!sameFormat)
        {
            RT_LOG_ERROR("Unsupported EXR format. All channels must be of the same type");
            goto exrImageError;
        }

        if (exrHeader.pixel_types[0] == TINYEXR_PIXELTYPE_FLOAT)
        {
            initData.format = Format::R32G32B32_Float;
            if (!Init(initData))
            {
                goto exrImageError;
            }

            for (size_t y = 0; y < (size_t)exrImage.height; ++y)
            {
                float* typedData = reinterpret_cast<float*>(mData + (size_t)mStride * y);
                for (size_t x = 0; x < (size_t)exrImage.width; ++x)
                {
                    const size_t index = y * exrImage.width + x;
                    typedData[3 * x    ] = reinterpret_cast<const float*>(exrImage.images[2])[index];
                    typedData[3 * x + 1] = reinterpret_cast<const float*>(exrImage.images[1])[index];
                    typedData[3 * x + 2] = reinterpret_cast<const float*>(exrImage.images[0])[index];
                }
            }
        }
        else if (exrHeader.pixel_types[0] == TINYEXR_PIXELTYPE_HALF)
        {
            initData.format = Format::R16G16B16_Half;
            if (!Init(initData))
            {
                goto exrImageError;
            }

            for (size_t y = 0; y < (size_t)exrImage.height; ++y)
            {
                Uint16* typedData = reinterpret_cast<Uint16*>(mData + (size_t)mStride * y);
                for (size_t x = 0; x < (size_t)exrImage.width; ++x)
                {
                    const size_t index = y * exrImage.width + x;
                    typedData[3 * x    ] = reinterpret_cast<const Uint16*>(exrImage.images[2])[index];
                    typedData[3 * x + 1] = reinterpret_cast<const Uint16*>(exrImage.images[1])[index];
                    typedData[3 * x + 2] = reinterpret_cast<const Uint16*>(exrImage.images[0])[index];
                }
            }
        }
        else
        {
            RT_LOG_ERROR("Unsupported EXR format: %i", exrHeader.pixel_types[0]);
            goto exrImageError;
        }
    }
    else if (exrHeader.num_channels == 4)
    {
        const bool sameFormat = exrHeader.pixel_types[0] == exrHeader.pixel_types[1]
            && exrHeader.pixel_types[0] == exrHeader.pixel_types[2]
            && exrHeader.pixel_types[0] == exrHeader.pixel_types[3];
        if (!sameFormat)
        {
            RT_LOG_ERROR("Unsupported EXR format. All channels must be of the same type");
            goto exrImageError;
        }

        if (exrHeader.pixel_types[0] == TINYEXR_PIXELTYPE_FLOAT)
        {
            initData.format = Format::R32G32B32A32_Float;
            if (!Init(initData))
            {
                goto exrImageError;
            }

            for (size_t y = 0; y < (size_t)exrImage.height; ++y)
            {
                float* typedData = reinterpret_cast<float*>(mData + (size_t)mStride * y);
                for (size_t x = 0; x < (size_t)exrImage.width; ++x)
                {
                    const size_t index = y * exrImage.width + x;
                    typedData[4 * x    ] = reinterpret_cast<const float*>(exrImage.images[3])[index];
                    typedData[4 * x + 1] = reinterpret_cast<const float*>(exrImage.images[2])[index];
                    typedData[4 * x + 2] = reinterpret_cast<const float*>(exrImage.images[1])[index];
                    typedData[4 * x + 3] = reinterpret_cast<const float*>(exrImage.images[0])[index];
                }
            }
        }
        else if (exrHeader.pixel_types[0] == TINYEXR_PIXELTYPE_HALF)
        {
            initData.format = Format::R16G16B16A16_Half;
            if (!Init(initData))
            {
                goto exrImageError;
            }

            for (size_t y = 0; y < (size_t)exrImage.height; ++y)
            {
                Uint16* typedData = reinterpret_cast<Uint16*>(mData + (size_t)mStride * y);
                for (size_t x = 0; x < (size_t)exrImage.width; ++x)
                {
                    const size_t index = y * exrImage.width + x;
                    typedData[4 * x    ] = reinterpret_cast<const Uint16*>(exrImage.images[3])[index];
                    typedData[4 * x + 1] = reinterpret_cast<const Uint16*>(exrImage.images[2])[index];
                    typedData[4 * x + 2] = reinterpret_cast<const Uint16*>(exrImage.images[1])[index];
                    typedData[4 * x + 3] = reinterpret_cast<const Uint16*>(exrImage.images[0])[index];
                }
            }
        }
        else
        {
            RT_LOG_ERROR("Unsupported EXR format: %i", exrHeader.pixel_types[0]);
            goto exrImageError;
        }
    }
    else
    {
        RT_LOG_ERROR("Unsupported EXR format.", path);
        goto exrImageError;
    }

    // 4. Free image data
    FreeEXRImage(&exrImage);
    return true;

exrImageError:
    FreeEXRImage(&exrImage);
    return false;
}

bool Bitmap::SaveEXR(const char* path, const float exposure) const
{
    if (mFormat != Format::R32G32B32_Float)
    {
        RT_LOG_ERROR("Bitmap::SaveEXR: Unsupported format");
        return false;
    }

    // TODO support more types

    const Float3* data = reinterpret_cast<const Float3*>(mData);

    EXRHeader header;
    InitEXRHeader(&header);

    EXRImage image;
    InitEXRImage(&image);

    image.num_channels = 3;

    DynArray<float> images[3];
    images[0].Resize(mWidth * mHeight);
    images[1].Resize(mWidth * mHeight);
    images[2].Resize(mWidth * mHeight);

    // Split RGBRGBRGB... into R, G and B layer
    const Uint32 numPixels = GetWidth() * GetHeight();
    for (Uint32 i = 0; i < numPixels; i++)
    {
        images[0][i] = exposure * data[i].x;
        images[1][i] = exposure * data[i].y;
        images[2][i] = exposure * data[i].z;
    }

    float* image_ptr[3];
    image_ptr[0] = images[2].Data(); // B
    image_ptr[1] = images[1].Data(); // G
    image_ptr[2] = images[0].Data(); // R

    image.images = (unsigned char**)image_ptr;
    image.width = mWidth;
    image.height = mHeight;

    header.compression_type = TINYEXR_COMPRESSIONTYPE_PIZ;
    header.num_channels = 3;
    header.channels = (EXRChannelInfo*)malloc(sizeof(EXRChannelInfo) * header.num_channels);

    // Must be (A)BGR order, since most of EXR viewers expect this channel order.
    {
        strcpy(header.channels[0].name, "B");
        strcpy(header.channels[1].name, "G");
        strcpy(header.channels[2].name, "R");
    }

    header.pixel_types = (int*)malloc(sizeof(int) * header.num_channels);
    header.requested_pixel_types = (int*)malloc(sizeof(int) * header.num_channels);
    for (int i = 0; i < header.num_channels; i++)
    {
        header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; // pixel type of input image
        header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; // pixel type of output image to be stored in .EXR
    }

    const char* err = nullptr;
    int ret = SaveEXRImageToFile(&image, &header, path, &err);
    if (ret != TINYEXR_SUCCESS)
    {
        RT_LOG_ERROR("Failed to save EXR file '%s': %s", path, err);
        FreeEXRErrorMessage(err);

        free(header.channels);
        free(header.pixel_types);
        free(header.requested_pixel_types);

        return ret;
    }

    RT_LOG_INFO("Image file '%s' written successfully", path);

    free(header.channels);
    free(header.pixel_types);
    free(header.requested_pixel_types);

    return true;
}

} // namespace rt
