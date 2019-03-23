#pragma once

#include "Texture.h"

#include "../Math/VectorInt4.h"
#include "../Utils/AlignmentAllocator.h"

namespace rt {

/**
 * Class representing 2D bitmap texture.
 */
class RT_ALIGN(16) Bitmap : public ITexture, public Aligned<16>
{
public:
    enum class Format : Uint8
    {
        Unknown = 0,
        R8_UNorm,
        B8G8R8_UNorm,
        B8G8R8A8_UNorm,
        R16G16B16A16_UNorm,
        R32G32B32_Float,
        R32G32B32A32_Float,
        R16G16B16_Half,
        R16G16B16A16_Half,
        BC1,
        BC4,
        BC5,

        // TODO monochromatic, compressed, half-float, etc.
    };

    RAYLIB_API Bitmap(const char* debugName = "<unnamed>");
    RAYLIB_API ~Bitmap();
    RAYLIB_API Bitmap(Bitmap&&);
    RAYLIB_API Bitmap& operator = (Bitmap&&);

    const char* GetName() const override;

    template<typename T>
    RT_FORCE_INLINE T* GetDataAs()
    {
        // TODO validate type
        return reinterpret_cast<T*>(GetData());
    }

    template<typename T>
    RT_FORCE_INLINE const T* GetDataAs() const
    {
        // TODO validate type
        return reinterpret_cast<const T*>(GetData());
    }

    RT_FORCE_INLINE void* GetData() { return mData; }
    RT_FORCE_INLINE const void* GetData() const { return mData; }
    RT_FORCE_INLINE Uint32 GetWidth() const { return (Uint32)mWidth; }
    RT_FORCE_INLINE Uint32 GetHeight() const { return (Uint32)mHeight; }
    RT_FORCE_INLINE Format GetFormat() const { return mFormat; }

    static size_t GetDataSize(Uint32 width, Uint32 height, Format format);

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

    // evaluate the bitmap color (including filtering and coordinates wrapping)
    virtual const math::Vector4 Evaluate(math::Vector4 coords, const TextureEvaluator& evaluator) const override;

    // fill with zeros
    RAYLIB_API void Clear();

private:

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
    Format mFormat;
    bool mLinearSpace;

    std::string mDebugName;
};

using BitmapPtr = std::shared_ptr<Bitmap>;

} // namespace rt
