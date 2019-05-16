#pragma once

#include "../Math/VectorInt4.h"
#include "../Utils/AlignmentAllocator.h"

namespace rt {

/**
 * Class representing 2D bitmap.
 */
class RT_ALIGN(16) Bitmap : public Aligned<16>
{
public:
    enum class Format : Uint8
    {
        Unknown = 0,
        R8_UNorm,
        R8G8_UNorm,
        B8G8R8_UNorm,
        B8G8R8A8_UNorm,
        R16_UNorm,
        R16G16_UNorm,
        R16G16B16A16_UNorm,
        R32_Float,
        R32G32B32_Float,
        R32G32B32A32_Float,
        R16_Half,
        R16G16_Half,
        R16G16B16_Half,
        R16G16B16A16_Half,
        BC1,
        BC4,
        BC5,
    };

    RAYLIB_API Bitmap(const char* debugName = "<unnamed>");
    RAYLIB_API ~Bitmap();
    RAYLIB_API Bitmap(Bitmap&&);
    RAYLIB_API Bitmap& operator = (Bitmap&&);

    RT_FORCE_INLINE const char* GetDebugName() const { return mDebugName; }

    template<typename T>
    RT_FORCE_INLINE T& GetPixelRef(Uint32 x, Uint32 y)
    {
        RT_ASSERT(x < mWidth && y < mHeight);
        RT_ASSERT(BitsPerPixel(mFormat) / 8 == sizeof(T));

        const size_t rowOffset = static_cast<size_t>(mStride) * static_cast<size_t>(y);
        return *reinterpret_cast<T*>(mData + rowOffset + sizeof(T) * x);
    }

    template<typename T>
    RT_FORCE_INLINE const T& GetPixelRef(Uint32 x, Uint32 y) const
    {
        RT_ASSERT(x < mWidth && y < mHeight);
        RT_ASSERT(BitsPerPixel(mFormat) / 8 == sizeof(T));

        const size_t rowOffset = static_cast<size_t>(mStride) * static_cast<size_t>(y);
        return *reinterpret_cast<const T*>(mData + rowOffset + sizeof(T) * x);
    }

    /*
    template<typename T>
    RT_FORCE_INLINE const T* GetDataAs() const
    {
        // TODO validate type
        return reinterpret_cast<const T*>(GetData());
    }

    template<typename T>
    RT_FORCE_INLINE T* GetDataAs()
    {
        // TODO validate type
        return reinterpret_cast<T*>(GetData());
    }
    */

    RT_FORCE_INLINE void* GetData() { return mData; }
    RT_FORCE_INLINE const void* GetData() const { return mData; }
    RT_FORCE_INLINE Uint32 GetWidth() const { return mWidth; }
    RT_FORCE_INLINE Uint32 GetStride() const { return mStride; }
    RT_FORCE_INLINE Uint32 GetHeight() const { return mHeight; }
    RT_FORCE_INLINE Format GetFormat() const { return mFormat; }

    static size_t ComputeDataSize(Uint32 width, Uint32 height, Format format);
    static Uint32 ComputeDataStride(Uint32 width, Format format);

    // initialize bitmap with data (or clean if passed nullptr)
    RAYLIB_API bool Init(Uint32 width, Uint32 height, Format format, const void* data = nullptr, bool linearSpace = false);

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
    static Uint32 BitsPerPixel(Format format);

    // get bitmap format description
    static const char* FormatToString(Format format);

    // get single pixel
    RAYLIB_API math::Vector4 GetPixel(Uint32 x, Uint32 y, const bool forceLinearSpace = false) const;

    // get 2x2 pixel block
    void GetPixelBlock(const math::VectorInt4 coords, const bool forceLinearSpace, math::Vector4* outColors) const;

    // fill with zeros
    RAYLIB_API void Clear();
    
    bool GaussianBlur(const float sigma, const Uint32 n);

private:

    friend class BitmapTexture;

    Bitmap(const Bitmap&) = delete;
    Bitmap& operator = (const Bitmap&) = delete;

    bool LoadBMP(FILE* file, const char* path);
    bool LoadDDS(FILE* file, const char* path);
    bool LoadEXR(FILE* file, const char* path);

    math::Vector4 mFloatSize = math::Vector4::Zero();
    math::VectorInt4 mSize = math::VectorInt4::Zero();
    Uint8* mData;
    Uint32 mWidth;
    Uint32 mHeight;
    Uint32 mStride;
    Format mFormat;
    bool mLinearSpace;
    char* mDebugName;
};

using BitmapPtr = std::shared_ptr<Bitmap>;

} // namespace rt
