#include "PCH.h"
#include "../Core/Math/VectorInt8.h"

#include "gtest/gtest.h"

using namespace rt::math;

//////////////////////////////////////////////////////////////////////////

TEST(MathTest, VectorInt8_Constructor1)
{
    const VectorInt8 v(1, 2, 3, 4, 5, 6, 7, 8);
    EXPECT_EQ(1, v[0]);
    EXPECT_EQ(2, v[1]);
    EXPECT_EQ(3, v[2]);
    EXPECT_EQ(4, v[3]);
    EXPECT_EQ(5, v[4]);
    EXPECT_EQ(6, v[5]);
    EXPECT_EQ(7, v[6]);
    EXPECT_EQ(8, v[7]);
}

TEST(MathTest, VectorInt8_Constructor2)
{
    const VectorInt8 v(42);
    EXPECT_EQ(42, v[0]);
    EXPECT_EQ(42, v[1]);
    EXPECT_EQ(42, v[2]);
    EXPECT_EQ(42, v[3]);
    EXPECT_EQ(42, v[4]);
    EXPECT_EQ(42, v[5]);
    EXPECT_EQ(42, v[6]);
    EXPECT_EQ(42, v[7]);
}

TEST(MathTest, VectorInt8_Zero)
{
    const VectorInt8 v = VectorInt8::Zero();
    EXPECT_EQ(0, v[0]);
    EXPECT_EQ(0, v[1]);
    EXPECT_EQ(0, v[2]);
    EXPECT_EQ(0, v[3]);
    EXPECT_EQ(0, v[4]);
    EXPECT_EQ(0, v[5]);
    EXPECT_EQ(0, v[6]);
    EXPECT_EQ(0, v[7]);
}

//////////////////////////////////////////////////////////////////////////

TEST(MathTest, VectorInt8_VectorArithmetics)
{
    EXPECT_EQ(VectorInt8(11, 22, 33, 44, 55, 66, 77, 88), VectorInt8(1, 2, 3, 4, 5, 6, 7, 8) + VectorInt8(10, 20, 30, 40, 50, 60, 70, 80));
    EXPECT_EQ(VectorInt8(1, 2, 3, 4, 5, 6, 7, 8), VectorInt8(11, 22, 33, 44, 55, 66, 77, 88) - VectorInt8(10, 20, 30, 40, 50, 60, 70, 80));
    EXPECT_EQ(VectorInt8(10, 40, 90, 160, 250, 360, 490, 640), VectorInt8(1, 2, 3, 4, 5, 6, 7, 8) * VectorInt8(10, 20, 30, 40, 50, 60, 70, 80));
    EXPECT_EQ(VectorInt8(2, 4, 6, 8, 10, 12, 14, 16), VectorInt8(1, 2, 3, 4, 5, 6, 7, 8) * 2);
    EXPECT_EQ(VectorInt8(-1, -2, -3, -4, -5, -6, -7, -8), -VectorInt8(1, 2, 3, 4, 5, 6, 7, 8));
}
