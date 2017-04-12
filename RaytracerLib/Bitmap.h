#pragma once

#include "RayLib.h"
#include "Math/Vector.h"

namespace rt {

/**
 * Class representing 2D image/texture.
 */
class RAYLIB_API Bitmap
{
public:
    enum class Format
    {
        Unknown,
        R8G8B8A8_Uint,
        R32G32B32A32_Float,
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
    Bool Init(Uint32 width, Uint32 height, Format format, const void* data = nullptr);

    // release memory
    void Release();

    // calculate number of bits per pixel for given format
    static size_t BitsPerPixel(Format format);

    // set single pixel
    // TODO: this probably will be too slow
    RT_FORCE_NOINLINE void SetPixel(Uint32 x, Uint32 y, const math::Vector& value);

    // get single pixel
    // TODO: this probably will be too slow
    math::Vector GetPixel(Uint32 x, Uint32 y) const;

private:

    Bitmap(const Bitmap&) = delete;
    Bitmap& operator = (const Bitmap&) = delete;

    void* mData;
    Uint16 mWidth;
    Uint16 mHeight;
    Format mFormat;
};


} // namespace rt
