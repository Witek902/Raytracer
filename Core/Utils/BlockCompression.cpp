#include "PCH.h"
#include "Math/VectorInt4.h"

namespace rt {

using namespace math;

RT_FORCE_NOINLINE
const Vector4 DecodeBC1(const Uint8* data, Uint32 x, Uint32 y, const Uint32 width)
{
    // FPU version
    /*
    const Uint8 weights0[] = { 0, 3, 1, 2 };
    const Uint8 weights1[] = { 3, 0, 2, 1 };

    const Uint32 blocksInRow = width / 4; // TODO non-4-multiply width support
    const Uint32 blockX = x / 4u;
    const Uint32 blockY = y / 4u;

    // calculate position inside block
    x %= 4;
    y %= 4;

    const Uint16* blockData = reinterpret_cast<const Uint16*>(data + 8 * (blocksInRow * blockY + blockX));
    const Uint32 code = *reinterpret_cast<const Uint32*>(blockData + 2);

    const Uint16 color0 = blockData[0];
    const Uint32 blue0  = (color0 & 0x001Fu);
    const Uint32 green0 = (color0 & 0x07E0u) >> 5u;
    const Uint32 red0   = (color0 & 0xF800u) >> 11u;

    const Uint16 color1 = blockData[1];
    const Uint32 blue1  = (color1 & 0x001Fu);
    const Uint32 green1 = (color1 & 0x07E0u) >> 5u;
    const Uint32 red1   = (color1 & 0xF800u) >> 11u;

    const Uint32 positionCode = (code >> 2u * (4u * y + x)) % 4;

    const Uint32 weight1 = weights0[positionCode];
    const Uint32 weight0 = weights1[positionCode];

    const Uint32 blue  = (weight0 * blue0 + weight1 * blue1) / 3u;
    const Uint32 green = (weight0 * green0 + weight1 * green1) / 3u;
    const Uint32 red   = (weight0 * red0 + weight1 * red1) / 3u;
    const Uint32 rgba =  ((blue << 19) | (green << 10)) | (red << 3);

    return Vector4::Load4((const Uint8*)&rgba) * (1.0f / 255.0f);
    */

    const size_t blocksInRow = width / 4u; // TODO non-4-multiply width support
    const size_t blockX = x / 4u;
    const size_t blockY = y / 4u;

    // calculate position inside block
    x %= 4u;
    y %= 4u;

    const Uint8* blockData = data + 8u * (blocksInRow * blockY + blockX);

    // extract base colors for given block
    const VectorInt4 mask = { 0x1F << 11, 0x3F << 5, 0x1F, 0 };
    const VectorInt4 raw0 = VectorInt4(*reinterpret_cast<const Int32*>(blockData + 0)) & mask;
    const VectorInt4 raw1 = VectorInt4(*reinterpret_cast<const Int32*>(blockData + 2)) & mask;
    const Vector4 color0 = raw0.ConvertToFloat();
    const Vector4 color1 = raw1.ConvertToFloat();
    // TODO alpha support

    // extract color index for given pixel
    const Uint32 code = *reinterpret_cast<const Uint32*>(blockData + 4);
    const Uint32 codeOffset = 2u * (4u * y + x);
    const Uint32 index = (code >> codeOffset) % 4;

    // calculate final color by blending base colors + scale down from 5,6,5 bit ranges to 0...1 float range
    const float weights[] = { 0.0f, 1.0f, 1.0f / 3.0f, 2.0f / 3.0f };
    const Vector4 scale{ 1.0f / 2048.0f / 31.0f, 1.0f / 32.0f / 63.0f, 1.0f / 31.0f, 0.0f };
    return Vector4::Lerp(color0, color1, weights[index]) * scale;
}

namespace helper
{

static float DecodeBC_Grayscale(const Uint8* blockData, const Uint32 x, const Uint32 y)
{
    const float intColor0 = blockData[0];
    const float intColor1 = blockData[1];
    const float color0 = static_cast<float>(intColor0) / 255.0f;
    const float color1 = static_cast<float>(intColor1) / 255.0f;

    const Uint64 code = *reinterpret_cast<const Uint64*>(blockData + 2);
    const Uint32 index = (code >> (Uint64)(3 * (4 * y + x))) % 8;

    // TODO SSE

    if (index == 0)
        return color0;
    else if (index == 1)
        return color1;
    else if (intColor0 > intColor1)
        return (color0 * (8 - index) + color1 * (index - 1)) / 7.0f;
    else
    {
        if (index == 6)
            return 0.0f;
        else if (index == 7)
            return 1.0f;
        else
            return (color0 * (6 - index) + color1 * (index - 1)) / 5.0f;
    }
}

} // helper

const Vector4 DecodeBC4(const Uint8* data, Uint32 x, Uint32 y, const Uint32 width)
{
    const size_t blocksInRow = width / 4; // TODO non-4-multiply width support
    const size_t blockX = x / 4u;
    const size_t blockY = y / 4u;

    // calculate position inside block
    x %= 4;
    y %= 4;

    const Uint8* blockData = reinterpret_cast<const Uint8*>(data + 8 * (blocksInRow * blockY + blockX));
    const float value = helper::DecodeBC_Grayscale(blockData, x, y);
    return Vector4(value, value, value, 1.0f);
}

const Vector4 DecodeBC5(const Uint8* data, Uint32 x, Uint32 y, const Uint32 width)
{
    const size_t blocksInRow = width / 4; // TODO non-4-multiply width support
    const size_t blockX = x / 4u;
    const size_t blockY = y / 4u;

    // calculate position inside block
    x %= 4;
    y %= 4;

    const Uint8* blockDataRed = reinterpret_cast<const Uint8*>(data + 16 * (blocksInRow * blockY + blockX));
    const float red = helper::DecodeBC_Grayscale(blockDataRed, x, y);

    const Uint8* blockDataGreen = blockDataRed + 8;
    const float green = helper::DecodeBC_Grayscale(blockDataGreen, x, y);

    return Vector4(green, red, 0.0f, 1.0f);
}

} // namespace rt
