#pragma once

#include "../RayLib.h"
#include "../Math/Vector4.h"


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
    math::Vector4 borderColor;
    TextureAddressMode addressU = TextureAddressMode::Repeat;
    TextureAddressMode addressV = TextureAddressMode::Repeat;
    TextureFilterMode filter = TextureFilterMode::Bilinear;
};

/**
 * Class representing 2D image/texture.
 */
class RAYLIB_API Bitmap
{
public:
    static constexpr Uint32 MaxSize = 4 * 1024; // TODO

    enum class Format : Uint8
    {
        Unknown = 0,
        B8G8R8_Uint,
        B8G8R8A8_Uint,
        R32G32B32_Float,
        R32G32B32A32_Float,

        // TODO monochromatic, compressed, half-float, etc.
    };

    Bitmap();
    ~Bitmap();
    Bitmap(Bitmap&&) = default;
    Bitmap& operator = (Bitmap&&) = default;

    RT_FORCE_INLINE const void* GetData() const { return mData; }
    RT_FORCE_INLINE Uint32 GetWidth() const { return (Uint32)mWidth; }
    RT_FORCE_INLINE Uint32 GetHeight() const { return (Uint32)mHeight; }
    RT_FORCE_INLINE Format GetFormat() const { return mFormat; }

    // initialize bitmap with data (or clean if passed nullptr)
    Bool Init(Uint32 width, Uint32 height, Format format, const void* data = nullptr, bool linearSpace = false);

    // load from BMP file
    Bool Load(const char* path);

    // set to zero
    void Clear();

    // release memory
    void Release();

    // calculate number of bits per pixel for given format
    static size_t BitsPerPixel(Format format);

    // set single pixel
    // TODO: this probably will be too slow
    void SetPixel(Uint32 x, Uint32 y, const math::Vector4& value);

    // get single pixel
    // TODO: this probably will be too slow
    math::Vector4 GetPixel(Uint32 x, Uint32 y) const;

    // sample the bitmap (including filtering and coordinates wrapping)
    math::Vector4 Sample(math::Vector4 coords, const SamplerDesc& sampler) const;

    // write whole pixels row
    void WriteHorizontalLine(Uint32 y, const math::Vector4* values);
    void WriteVerticalLine(Uint32 x, const math::Vector4* values);

    // read whole pixels row
    void ReadHorizontalLine(Uint32 y, math::Vector4* outValues) const;
    void ReadVerticalLine(Uint32 x, math::Vector4* outValues) const;


    // fill with zeros
    void Zero();

    // fast Box blur
    static Bool VerticalBoxBlur(Bitmap& target, const Bitmap& src, const Uint32 radius);
    static Bool HorizontalBoxBlur(Bitmap& target, const Bitmap& src, const Uint32 radius);

    static Bool Blur(Bitmap& target, const Bitmap& src, const Float sigma, const Uint32 n);

private:

    Bitmap(const Bitmap&) = delete;
    Bitmap& operator = (const Bitmap&) = delete;

    bool LoadBMP(FILE* file, const char* path);
    bool LoadDDS(FILE* file, const char* path);

    static void BoxBlur_Internal(math::Vector4* targetLine, const math::Vector4* srcLine,
                                 const Uint32 radius, const Uint32 width, const Float factor);

    void* mData;
    Uint16 mWidth;
    Uint16 mHeight;
    Format mFormat;
    bool mLinearSpace;
};


} // namespace rt
