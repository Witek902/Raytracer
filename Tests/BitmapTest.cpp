#include "PCH.h"
#include "../Core/Utils/Bitmap.h"
#include "../Core/Math/Half.h"
#include "../Core/Math/Packed.h"

using namespace rt;
using namespace rt::math;

///////////////////////////////////////////////////////////////////////////////////////////////////

TEST(BitmapTest, Empty)
{
    Bitmap bitmap;

    EXPECT_EQ(0, bitmap.GetWidth());
    EXPECT_EQ(0, bitmap.GetHeight());
    EXPECT_EQ(0, bitmap.GetStride());
    EXPECT_EQ(nullptr, bitmap.GetData());
}

void CompareVector(const Vector4& ref, const Vector4& val, float maxError = 0.0f)
{
    const Vector4 diff = Vector4::Abs(ref - val);
    EXPECT_LE(diff.x, maxError);
    EXPECT_LE(diff.y, maxError);
    EXPECT_LE(diff.z, maxError);
    EXPECT_LE(diff.w, maxError);
}

static void Validate_GetPixel(const Bitmap& bitmap, const Vector4* expectedValues, float maxError = 0.0f, Uint32 width = 2, Uint32 height = 2)
{
    for (Uint32 y = 0; y < height; ++y)
    {
        SCOPED_TRACE("y=" + std::to_string(y));

        for (Uint32 x = 0; x < width; ++x)
        {
            SCOPED_TRACE("x=" + std::to_string(x));

            const Vector4& expected = expectedValues[y * width + x];
            const Vector4 actual = bitmap.GetPixel(x, y);

            EXPECT_NEAR(expected.x, actual.x, maxError);
            EXPECT_NEAR(expected.y, actual.y, maxError);
            EXPECT_NEAR(expected.z, actual.z, maxError);
            EXPECT_NEAR(expected.w, actual.w, maxError);
        }
    }
}

static void Validate_GetPixelBlock(const Bitmap& bitmap, const Vector4* expectedValues, float maxError = 0.0f, Uint32 x = 0, Uint32 y = 0)
{
    Vector4 actual[4];

    bitmap.GetPixelBlock(VectorInt4(x, y, x + 1, y + 1), actual);

    for (Uint32 i = 0; i < 4; ++i)
    {
        SCOPED_TRACE("i=" + std::to_string(i));

        const Vector4& expected = expectedValues[i];

        EXPECT_NEAR(expected.x, actual[i].x, maxError);
        EXPECT_NEAR(expected.y, actual[i].y, maxError);
        EXPECT_NEAR(expected.z, actual[i].z, maxError);
        EXPECT_NEAR(expected.w, actual[i].w, maxError);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

TEST(BitmapTest, Format_R8_UNorm)
{
    Bitmap bitmap;
    {
        const Uint8 data[] =
        {
            0,
            13,
            128,
            255,
        };
        ASSERT_TRUE(bitmap.Init({ 2, 2, Bitmap::Format::R8_UNorm, data }));
        ASSERT_EQ(2, bitmap.GetStride());
    }

    const Vector4 expected[] =
    {
        Vector4(0.0f),
        Vector4(13.0f / 255.0f),
        Vector4(128.0f / 255.0f),
        Vector4(1.0f),
    };
    Validate_GetPixel(bitmap, expected, 0.00001f);
    Validate_GetPixelBlock(bitmap, expected, 0.00001f);
}

TEST(BitmapTest, Format_R8G8_UNorm)
{
    Bitmap bitmap;
    {
        const Uint8 data[] =
        {
            0,      123,
            13,     0,
            128,    255,
            255,    13
        };
        ASSERT_TRUE(bitmap.Init({ 2, 2, Bitmap::Format::R8G8_UNorm, data }));
        ASSERT_EQ(4, bitmap.GetStride());
    }

    const Vector4 expected[] =
    {
        Vector4(0.0f, 123.0f / 255.0f),
        Vector4(13.0f / 255.0f, 0.0f),
        Vector4(128.0f / 255.0f, 1.0f),
        Vector4(1.0f, 13.0f / 255.0f),
    };
    Validate_GetPixel(bitmap, expected, 0.00001f);
    Validate_GetPixelBlock(bitmap, expected, 0.00001f);
}

TEST(BitmapTest, Format_B8G8R8_UNorm)
{
    Bitmap bitmap;
    {
        const Uint8 data[] =
        {
            0,      123,    17,
            13,     0,      255,
            128,    255,    201,
            255,    13,     0,
        };
        ASSERT_TRUE(bitmap.Init({ 2, 2, Bitmap::Format::B8G8R8_UNorm, data }));
        ASSERT_EQ(6, bitmap.GetStride());
    }

    const Vector4 expected[] =
    {
        Vector4(17.0f / 255.0f, 123.0f / 255.0f, 0.0f),
        Vector4(1.0f, 0.0f, 13.0f / 255.0f),
        Vector4(201.0f / 255.0f, 1.0f, 128.0f / 255.0f),
        Vector4(0.0f, 13.0f / 255.0f, 1.0f),
    };
    Validate_GetPixel(bitmap, expected, 0.00001f);
    Validate_GetPixelBlock(bitmap, expected, 0.00001f);
}

TEST(BitmapTest, Format_B8G8R8A8_UNorm)
{
    Bitmap bitmap;
    {
        const Uint8 data[] =
        {
            0,      123,    17,     255,
            13,     0,      255,    30,
            128,    255,    201,    0,
            255,    13,     0,      190,
        };
        ASSERT_TRUE(bitmap.Init({ 2, 2, Bitmap::Format::B8G8R8A8_UNorm, data }));
        ASSERT_EQ(8, bitmap.GetStride());
    }

    const Vector4 expected[] =
    {
        Vector4(17.0f / 255.0f, 123.0f / 255.0f, 0.0f, 1.0f),
        Vector4(1.0f, 0.0f, 13.0f / 255.0f, 30.0f / 255.0f),
        Vector4(201.0f / 255.0f, 1.0f, 128.0f / 255.0f, 0.0f),
        Vector4(0.0f, 13.0f / 255.0f, 1.0f, 190.0f / 255.0f),
    };
    Validate_GetPixel(bitmap, expected, 0.00001f);
    Validate_GetPixelBlock(bitmap, expected, 0.00001f);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

TEST(BitmapTest, Format_B5G6R5_UNorm)
{
    Bitmap bitmap;
    {
        const Packed565 data[] =
        {
            Packed565( 0, 59, 29),
            Packed565( 2,  0, 31),
            Packed565(29, 63,  2),
            Packed565(31,  7,  0),
        };
        ASSERT_TRUE(bitmap.Init({ 2, 2, Bitmap::Format::B5G6R5_UNorm, data }));
        ASSERT_EQ(4, bitmap.GetStride());
    }

    const Vector4 expected[] =
    {
        Vector4(29.0f / 31.0f, 59.0f / 63.0f, 0.0f / 31.0f),
        Vector4(31.0f / 31.0f, 0.0f / 63.0f, 2.0f / 31.0f),
        Vector4(2.0f / 31.0f, 63.0f / 63.0f, 29.0f / 31.0f),
        Vector4(0.0f / 31.0f, 7.0f / 63.0f, 31.0f / 31.0f),
    };
    Validate_GetPixel(bitmap, expected, 0.001f);
    Validate_GetPixelBlock(bitmap, expected, 0.001f);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

TEST(BitmapTest, Format_R16_UNorm)
{
    Bitmap bitmap;
    {
        const Uint16 data[] =
        {
            0,
            120,
            47813,
            65535,
        };
        ASSERT_TRUE(bitmap.Init({ 2, 2, Bitmap::Format::R16_UNorm, data }));
        ASSERT_EQ(4, bitmap.GetStride());
    }

    const Vector4 expected[] =
    {
        Vector4(0.0f),
        Vector4(120.0f / 65535.0f),
        Vector4(47813.0f / 65535.0f),
        Vector4(1.0f),
    };
    Validate_GetPixel(bitmap, expected, 0.00001f);
    Validate_GetPixelBlock(bitmap, expected, 0.00001f);
}

TEST(BitmapTest, Format_R16G16_UNorm)
{
    Bitmap bitmap;
    {
        const Uint16 data[] =
        {
            0,      120,
            120,    0,
            47813,  65535,
            65535,  47813,
        };
        ASSERT_TRUE(bitmap.Init({ 2, 2, Bitmap::Format::R16G16_UNorm, data }));
        ASSERT_EQ(8, bitmap.GetStride());
    }

    const Vector4 expected[] =
    {
        Vector4(0.0f, 120.0f) / 65535.0f,
        Vector4(120.0f, 0.0f) / 65535.0f,
        Vector4(47813.0f, 65535.0f) / 65535.0f,
        Vector4(65535.0f, 47813.0f) / 65535.0f,
    };
    Validate_GetPixel(bitmap, expected, 0.00001f);
    Validate_GetPixelBlock(bitmap, expected, 0.00001f);
}

TEST(BitmapTest, Format_R16G16B16A16_UNorm)
{
    Bitmap bitmap;
    {
        const Uint16 data[] =
        {
            0,      120,    47813,  65535,
            120,    0,      65535,  47813,
            47813,  65535,  120,    0,
            65535,  47813,  0,      120,
        };
        ASSERT_TRUE(bitmap.Init({ 2, 2, Bitmap::Format::R16G16B16A16_UNorm, data }));
        ASSERT_EQ(16, bitmap.GetStride());
    }

    const Vector4 expected[] =
    {
        Vector4(0.0f, 120.0f, 47813.0f, 65535.0f) / 65535.0f,
        Vector4(120.0f, 0.0f, 65535.0f, 47813.0f) / 65535.0f,
        Vector4(47813.0f, 65535.0f, 120.0f, 0.0f) / 65535.0f,
        Vector4(65535.0f, 47813.0f, 0.0f, 120.0f) / 65535.0f,
    };
    Validate_GetPixel(bitmap, expected, 0.00001f);
    Validate_GetPixelBlock(bitmap, expected, 0.00001f);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

TEST(BitmapTest, Format_R32_Float)
{
    Bitmap bitmap;
    {
        const float data[] =
        {
            0.0f,
            -123.0f,
            0.2f,
            10000.0f
        };
        ASSERT_TRUE(bitmap.Init({ 2, 2, Bitmap::Format::R32_Float, data }));
        ASSERT_EQ(8, bitmap.GetStride());
    }

    const Vector4 expected[] =
    {
        Vector4(0.0f),
        Vector4(-123.0f),
        Vector4(0.2f),
        Vector4(10000.0f),
    };
    Validate_GetPixel(bitmap, expected, 0.00001f);
    Validate_GetPixelBlock(bitmap, expected, 0.00001f);
}

TEST(BitmapTest, Format_R32G32_Float)
{
    Bitmap bitmap;
    {
        const float data[] =
        {
            0.0f,       123.0f,
            -123.0f,    0.0f,
            0.2f,       1000.0f,
            10000.0f,   0.25f,
        };
        ASSERT_TRUE(bitmap.Init({ 2, 2, Bitmap::Format::R32G32_Float, data }));
        ASSERT_EQ(16, bitmap.GetStride());
    }

    const Vector4 expected[] =
    {
        Vector4(0.0f, 123.0f),
        Vector4(-123.0f, 0.0f),
        Vector4(0.2f, 1000.0f),
        Vector4(10000.0f, 0.25f),
    };
    Validate_GetPixel(bitmap, expected, 0.00001f);
    Validate_GetPixelBlock(bitmap, expected, 0.00001f);
}

TEST(BitmapTest, Format_R32G32B32_Float)
{
    Bitmap bitmap;
    {
        const float data[] =
        {
            0.0f,       123.0f,     100.0f,
            -123.0f,    0.0f,       0.1f,
            0.2f,       1000.0f,    0.0f,
            10000.0f,   0.25f,      -10.0f,
        };
        ASSERT_TRUE(bitmap.Init({ 2, 2, Bitmap::Format::R32G32B32_Float, data }));
        ASSERT_EQ(24, bitmap.GetStride());
    }

    const Vector4 expected[] =
    {
        Vector4(0.0f, 123.0f, 100.0f),
        Vector4(-123.0f, 0.0f, 0.1f),
        Vector4(0.2f, 1000.0f, 0.0f),
        Vector4(10000.0f, 0.25f, -10.0f),
    };
    Validate_GetPixel(bitmap, expected, 0.00001f);
    Validate_GetPixelBlock(bitmap, expected, 0.00001f);
}

TEST(BitmapTest, Format_R32G32B32A32_Float)
{
    Bitmap bitmap;
    {
        const float data[] =
        {
            0.0f,       123.0f,     100.0f,     -0.1f,
            -123.0f,    0.0f,       0.1f,       200.0f,
            0.2f,       1000.0f,    0.0f,       0.8f,
            10000.0f,   0.25f,      -10.0f,     0.0f,
        };
        ASSERT_TRUE(bitmap.Init({ 2, 2, Bitmap::Format::R32G32B32A32_Float, data }));
        ASSERT_EQ(32, bitmap.GetStride());
    }

    const Vector4 expected[] =
    {
        Vector4(0.0f, 123.0f, 100.0f, -0.1f),
        Vector4(-123.0f, 0.0f, 0.1f, 200.0f),
        Vector4(0.2f, 1000.0f, 0.0f, 0.8f),
        Vector4(10000.0f, 0.25f, -10.0f, 0.0f),
    };
    Validate_GetPixel(bitmap, expected, 0.00001f);
    Validate_GetPixelBlock(bitmap, expected, 0.00001f);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

TEST(BitmapTest, Format_R16_Half)
{
    Bitmap bitmap;
    {
        const Half data[] =
        {
            0.0f,
            -123.0f,
            0.2f,
            1000.0f
        };
        ASSERT_TRUE(bitmap.Init({ 2, 2, Bitmap::Format::R16_Half, data }));
        ASSERT_EQ(4, bitmap.GetStride());
    }

    const Vector4 expected[] =
    {
        Vector4(0.0f),
        Vector4(-123.0f),
        Vector4(0.2f),
        Vector4(1000.0f),
    };
    Validate_GetPixel(bitmap, expected, 0.001f);
    Validate_GetPixelBlock(bitmap, expected, 0.001f);
}

TEST(BitmapTest, Format_R16G16_Half)
{
    Bitmap bitmap;
    {
        const Half data[] =
        {
            0.0f,       123.0f,
            -123.0f,    0.0f,
            0.2f,       1000.0f,
            1000.0f,    0.25f,
        };
        ASSERT_TRUE(bitmap.Init({ 2, 2, Bitmap::Format::R16G16_Half, data }));
        ASSERT_EQ(8, bitmap.GetStride());
    }

    const Vector4 expected[] =
    {
        Vector4(0.0f, 123.0f),
        Vector4(-123.0f, 0.0f),
        Vector4(0.2f, 1000.0f),
        Vector4(1000.0f, 0.25f),
    };
    Validate_GetPixel(bitmap, expected, 0.001f);
    Validate_GetPixelBlock(bitmap, expected, 0.001f);
}

TEST(BitmapTest, Format_R16G16B16_Half)
{
    Bitmap bitmap;
    {
        const Half data[] =
        {
            0.0f,       123.0f,     100.0f,
            -123.0f,    0.0f,       0.1f,
            0.2f,       1000.0f,    0.0f,
            1000.0f,    0.25f,      -10.0f,
        };
        ASSERT_TRUE(bitmap.Init({ 2, 2, Bitmap::Format::R16G16B16_Half, data }));
        ASSERT_EQ(12, bitmap.GetStride());
    }

    const Vector4 expected[] =
    {
        Vector4(0.0f, 123.0f, 100.0f),
        Vector4(-123.0f, 0.0f, 0.1f),
        Vector4(0.2f, 1000.0f, 0.0f),
        Vector4(1000.0f, 0.25f, -10.0f),
    };
    Validate_GetPixel(bitmap, expected, 0.001f);
    Validate_GetPixelBlock(bitmap, expected, 0.001f);
}

TEST(BitmapTest, Format_R16G16B16A16_Half)
{
    Bitmap bitmap;
    {
        const Half data[] =
        {
            0.0f,       123.0f,     100.0f,     -0.1f,
            -123.0f,    0.0f,       0.1f,       200.0f,
            0.2f,       1000.0f,    0.0f,       0.8f,
            1000.0f,    0.25f,      -10.0f,     0.0f,
        };
        ASSERT_TRUE(bitmap.Init({ 2, 2, Bitmap::Format::R16G16B16A16_Half, data }));
        ASSERT_EQ(16, bitmap.GetStride());
    }

    const Vector4 expected[] =
    {
        Vector4(0.0f, 123.0f, 100.0f, -0.1f),
        Vector4(-123.0f, 0.0f, 0.1f, 200.0f),
        Vector4(0.2f, 1000.0f, 0.0f, 0.8f),
        Vector4(1000.0f, 0.25f, -10.0f, 0.0f),
    };
    Validate_GetPixel(bitmap, expected, 0.001f);
    Validate_GetPixelBlock(bitmap, expected, 0.001f);
}
