#include "PCH.h"
#include "../Core/Math/Vector4Load.h"

using namespace rt::math;

TEST(MathTest, Vector4_Load_2xUint8_Norm)
{
    {
        const uint8 data[2] = { 0, 0 };
        EXPECT_TRUE((Vector4(0.0f, 0.0f, 0.0f, 0.0f) == Vector4_Load_2xUint8_Norm(data)).All());
    }
    {
        const uint8 data[2] = { 0, 255 };
        EXPECT_TRUE((Vector4(0.0f, 1.0f, 0.0f, 0.0f) == Vector4_Load_2xUint8_Norm(data)).All());
    }
    {
        const uint8 data[2] = { 255, 255 };
        EXPECT_TRUE((Vector4(1.0f, 1.0f, 0.0f, 0.0f) == Vector4_Load_2xUint8_Norm(data)).All());
    }
    {
        const uint8 data[2] = { 255, 0 };
        EXPECT_TRUE((Vector4(1.0f, 0.0f, 0.0f, 0.0f) == Vector4_Load_2xUint8_Norm(data)).All());
    }
    {
        const uint8 data[2] = { 35, 86 };
        EXPECT_TRUE((Vector4(35.0f / 255.0f, 86.0f / 255.0f, 0.0f, 0.0f) == Vector4_Load_2xUint8_Norm(data)).All());
    }
}

TEST(MathTest, Vector4_Load_4xUint8_Norm)
{
    {
        const uint8 data[4] = { 0, 0, 0, 0 };
        EXPECT_TRUE((Vector4(0.0f, 0.0f, 0.0f, 0.0f) == Vector4_Load_4xUint8(data)).All());
    }
    {
        const uint8 data[4] = { 255, 0, 0, 255 };
        EXPECT_TRUE((Vector4(255.0f, 0.0f, 0.0f, 255.0f) == Vector4_Load_4xUint8(data)).All());
    }
    {
        const uint8 data[4] = { 255, 255, 255, 255 };
        EXPECT_TRUE((Vector4(255.0f, 255.0f, 255.0f, 255.0f) == Vector4_Load_4xUint8(data)).All());
    }
    {
        const uint8 data[4] = { 35, 86, 241, 13 };
        EXPECT_TRUE((Vector4(35.0f, 86.0f, 241.0f, 13.0f) == Vector4_Load_4xUint8(data)).All());
    }
}

TEST(MathTest, Vector4_Load_2xUint16_Norm)
{
    {
        const uint16 data[2] = { 0, 0 };
        EXPECT_TRUE((Vector4(0.0f, 0.0f, 0.0f, 0.0f) == Vector4_Load_2xUint16_Norm(data)).All());
    }
    {
        const uint16 data[2] = { 0, 65535 };
        EXPECT_TRUE((Vector4(0.0f, 1.0f, 0.0f, 0.0f) == Vector4_Load_2xUint16_Norm(data)).All());
    }
    {
        const uint16 data[2] = { 65535, 0 };
        EXPECT_TRUE((Vector4(1.0f, 0.0f, 0.0f, 0.0f) == Vector4_Load_2xUint16_Norm(data)).All());
    }
    {
        const uint16 data[2] = { 65535, 65535 };
        EXPECT_TRUE((Vector4(1.0f, 1.0f, 0.0f, 0.0f) == Vector4_Load_2xUint16_Norm(data)).All());
    }
    {
        const uint16 data[2] = { 31351, 8135 };
        EXPECT_TRUE((Vector4(31351.0f / 65535.0f, 8135.0f / 65535.0f, 0.0f, 0.0f) == Vector4_Load_2xUint16_Norm(data)).All());
    }
}

TEST(MathTest, Vector4_Load_4xUint16)
{
    {
        const uint16 data[4] = { 0, 0, 0, 0 };
        EXPECT_TRUE((Vector4(0.0f, 0.0f, 0.0f, 0.0f) == Vector4_Load_4xUint16(data)).All());
    }
    {
        const uint16 data[4] = { 65535, 0, 0, 65535 };
        EXPECT_TRUE((Vector4(65535.0f, 0.0f, 0.0f, 65535.0f) == Vector4_Load_4xUint16(data)).All());
    }
    {
        const uint16 data[4] = { 65535, 65535, 65535, 65535 };
        EXPECT_TRUE((Vector4(65535.0f, 65535.0f, 65535.0f, 65535.0f) == Vector4_Load_4xUint16(data)).All());
    }
    {
        const uint16 data[4] = { 31351, 8135, 12, 57964 };
        EXPECT_TRUE((Vector4(31351.0f, 8135.0f, 12.0f, 57964.0f) == Vector4_Load_4xUint16(data)).All());
    }
}
