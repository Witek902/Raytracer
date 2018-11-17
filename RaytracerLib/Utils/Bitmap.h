#pragma once

#include "../RayLib.h"
#include "../Math/Vector4.h"
#include "../Math/Vector8.h"
#include "../Utils/AlignmentAllocator.h"

namespace rt {

enum class TextureAddressMode : Uint8
{
    Repeat = 0,
    Clamp = 1,
    Border = 2,
};

enum class TextureFilterMode : Uint8
{
    NearestNeighbor = 0,
    Bilinear = 1,
};

struct SamplerDesc
{
    math::Vector4 borderColor = math::Vector4::Zero();
    TextureAddressMode addressU = TextureAddressMode::Repeat;
    TextureAddressMode addressV = TextureAddressMode::Repeat;
    TextureFilterMode filter = TextureFilterMode::Bilinear;
    bool forceLinearSpace = false;
};

/**
 * Class representing 2D image/texture.
 */
class RT_ALIGN(64) RAYLIB_API Bitmap : public Aligned<64>
{
public:
    enum class Format : Uint8
    {
        Unknown = 0,
        R8_Uint,
        B8G8R8_Uint,
        B8G8R8A8_Uint,
        R32G32B32_Float,
        R32G32B32A32_Float,
        R16G16B16_Half,
        BC1,
        BC4,
        BC5,

        // TODO monochromatic, compressed, half-float, etc.
    };

    Bitmap();
    ~Bitmap();
    Bitmap(Bitmap&&) = default;
    Bitmap& operator = (Bitmap&&) = default;

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
    Bool Init(Uint32 width, Uint32 height, Format format, const void* data = nullptr, bool linearSpace = false);

    // copy texture data
    // NOTE: both textures must have the same format and size
    static Bool Copy(Bitmap& target, const Bitmap& source);

    // load from file
    Bool Load(const char* path);

    // save to BMP file
    bool SaveBMP(const char* path, bool flipVertically) const;

    // save to OpenEXR file
    // NOTE: must be Float or Half format
    bool SaveEXR(const char* path, const Float exposure) const;

    // release memory
    void Release();

    // Convert to tiled texture
    bool MakeTiled(Uint8 order);

    // calculate number of bits per pixel for given format
    static Uint32 BitsPerPixel(Format format);

    // get bitmap format description
    static const char* FormatToString(Format format);

    // set single pixel
    // TODO: this probably will be too slow
    void SetPixel(Uint32 x, Uint32 y, const math::Vector4& value);

    // get single pixel
    // TODO: this probably will be too slow
    math::Vector4 GetPixel(Uint32 x, Uint32 y, const bool forceLinearSpace = false) const;

    // sample the bitmap (including filtering and coordinates wrapping)
    math::Vector4 Sample(math::Vector4 coords, const SamplerDesc& sampler) const;

    // fill with zeros
    void Clear();

private:

    Bitmap(const Bitmap&) = delete;
    Bitmap& operator = (const Bitmap&) = delete;

    bool LoadBMP(FILE* file, const char* path);
    bool LoadDDS(FILE* file, const char* path);
    bool LoadEXR(FILE* file, const char* path);

    math::Vector4 mSize = math::Vector4::Zero();
    Uint8* mData;
    Uint16 mWidth;
    Uint16 mHeight;
    Format mFormat;
    Uint8 mTileOrder;
    bool mLinearSpace;
};


} // namespace rt
