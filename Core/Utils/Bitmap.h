#pragma once

#include "../Math/VectorInt4.h"
#include "../Utils/Memory.h"

namespace rt {

/**
 * Class representing 2D bitmap.
 */
class RT_ALIGN(16) Bitmap : public Aligned<16>
{
public:

    enum class Format : uint8
    {
        Unknown = 0,
        R8_UNorm,
        R8G8_UNorm,
        B8G8R8_UNorm,
        B8G8R8A8_UNorm,
        R8G8B8A8_UNorm,
        B8G8R8A8_UNorm_Palette,
        B5G6R5_UNorm,
        R16_UNorm,
        R16G16_UNorm,
        R16G16B16A16_UNorm,
        R32_Float,
        R32G32_Float,
        R32G32B32_Float,
        R32G32B32A32_Float,
        R11G11B10_Float,
        R16_Half,
        R16G16_Half,
        R16G16B16_Half,
        R16G16B16A16_Half,
        R9G9B9E5_SharedExp,
        BC1,
        BC4,
        BC5,
    };

    struct InitData
    {
        uint32 width = 0;
        uint32 height = 0;
        Format format = Format::Unknown;
        const void* data = nullptr;
        uint32 stride = 0;
        bool linearSpace = true;
        uint32 paletteSize = 0;
        bool useDefaultAllocator = false;
    };

    RAYLIB_API Bitmap(const char* debugName = "<unnamed>");
    RAYLIB_API ~Bitmap();
    RAYLIB_API Bitmap(Bitmap&&);
    RAYLIB_API Bitmap& operator = (Bitmap&&);

    RT_FORCE_INLINE const char* GetDebugName() const { return mDebugName; }

    template<typename T>
    RT_FORCE_INLINE T& GetPixelRef(uint32 x, uint32 y)
    {
        RT_ASSERT(x < mWidth && y < mHeight);
        RT_ASSERT(BitsPerPixel(mFormat) / 8 == sizeof(T));

        const size_t rowOffset = static_cast<size_t>(mStride) * static_cast<size_t>(y);
        return *reinterpret_cast<T*>(mData + rowOffset + sizeof(T) * x);
    }

    template<typename T>
    RT_FORCE_INLINE const T& GetPixelRef(uint32 x, uint32 y) const
    {
        RT_ASSERT(x < mWidth && y < mHeight);
        RT_ASSERT(BitsPerPixel(mFormat) / 8 == sizeof(T));

        const size_t rowOffset = static_cast<size_t>(mStride) * static_cast<size_t>(y);
        return *reinterpret_cast<const T*>(mData + rowOffset + sizeof(T) * x);
    }

    RT_FORCE_INLINE uint8* GetData() { return mData; }
    RT_FORCE_INLINE const uint8* GetData() const { return mData; }
    RT_FORCE_INLINE uint32 GetWidth() const { return mWidth; }
    RT_FORCE_INLINE uint32 GetStride() const { return mStride; }
    RT_FORCE_INLINE uint32 GetHeight() const { return mHeight; }
    RT_FORCE_INLINE Format GetFormat() const { return mFormat; }

    // get allocated size
    RT_FORCE_INLINE size_t GetDataSize() const { return (size_t)mStride * (size_t)mHeight; }

    static size_t ComputeDataSize(const InitData& initData);
    static uint32 ComputeDataStride(uint32 width, Format format);

    // initialize bitmap with data (or clean if passed nullptr)
    RAYLIB_API bool Init(const InitData& initData);

    // copy texture data
    // NOTE: both textures must have the same format and size
    RAYLIB_API static bool Copy(Bitmap& target, const Bitmap& source);

    // load from file
    RAYLIB_API bool Load(const char* path);

    // save to BMP file
    RAYLIB_API bool SaveBMP(const char* path, bool flipVertically) const;

    // save to OpenEXR file
    // NOTE: must be float or Half format
    RAYLIB_API bool SaveEXR(const char* path, const float exposure) const;

    // release memory
    void Release();

    // calculate number of bits per pixel for given format
    static uint8 BitsPerPixel(Format format);

    // get bitmap format description
    static const char* FormatToString(Format format);

    // get single pixel
    RAYLIB_API const math::Vector4 GetPixel(uint32 x, uint32 y, const bool forceLinearSpace = false) const;

    // get 2x2 pixel block
    RAYLIB_API void GetPixelBlock(const math::VectorInt4 coords, math::Vector4* outColors, const bool forceLinearSpace = false) const;

    // fill with zeros
    RAYLIB_API void Clear();
    
    bool GaussianBlur(const float sigma, const uint32 n);

private:

    friend class BitmapTexture;

    Bitmap(const Bitmap&) = delete;
    Bitmap& operator = (const Bitmap&) = delete;

    bool LoadBMP(FILE* file, const char* path);
    bool LoadDDS(FILE* file, const char* path);
    bool LoadEXR(FILE* file, const char* path);

    math::Vector4 mFloatSize = math::Vector4::Zero();
    uint8* mData;
    uint8* mPalette;
    uint32 mWidth;          // number of pixels in a row
    uint32 mHeight;         // number of rows
    uint32 mStride;         // number of bytes between rows
    uint32 mPaletteSize;    // number of colors in the palette
    Format mFormat;
    bool mLinearSpace : 1;
    bool mUsesDefaultAllocator : 1;
    char* mDebugName;
};

using BitmapPtr = std::shared_ptr<Bitmap>;

} // namespace rt
