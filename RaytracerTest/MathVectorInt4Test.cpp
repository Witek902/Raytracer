#include "PCH.h"
#include "../RaytracerLib/Math/VectorInt4.h"

#include "gtest/gtest.h"

using namespace rt::math;

namespace {

static const VectorInt4 vecA = VectorInt4(1, 1, 1, 1);
static const VectorInt4 vecB = VectorInt4(1, 2, 3, 4);
static const VectorInt4 vecC = VectorInt4(2, 3, 4, 5);
static const VectorInt4 vecD = VectorInt4(1, 4, 9, 16);
static const VectorInt4 vecE = VectorInt4(4, 3, 2, 1);

} // namespace

//////////////////////////////////////////////////////////////////////////

TEST(MathTest, VectorInt4_Constructor1)
{
    const VectorInt4 v(1, 2, 3, 4);
    EXPECT_EQ(1, v.x);
    EXPECT_EQ(2, v.y);
    EXPECT_EQ(3, v.z);
    EXPECT_EQ(4, v.w);
}

TEST(MathTest, VectorInt4_Constructor2)
{
    const VectorInt4 v(7);
    EXPECT_EQ(7, v.x);
    EXPECT_EQ(7, v.y);
    EXPECT_EQ(7, v.z);
    EXPECT_EQ(7, v.w);
}

TEST(MathTest, VectorInt4_Zero)
{
    const VectorInt4 v = VectorInt4::Zero();
    EXPECT_EQ(0, v.x);
    EXPECT_EQ(0, v.y);
    EXPECT_EQ(0, v.z);
    EXPECT_EQ(0, v.w);
}

//////////////////////////////////////////////////////////////////////////

TEST(MathTest, VectorInt4_VectorArithmetics)
{
    EXPECT_EQ(vecC, vecA + vecB);
    EXPECT_EQ(VectorInt4(0, -1, -2, -3), vecA - vecB);
    EXPECT_EQ(vecD, vecB * vecB);
    EXPECT_EQ(VectorInt4(2, 4, 6, 8), vecB * 2);
    EXPECT_EQ(VectorInt4(-1, -2, -3, -4), -VectorInt4(1, 2, 3, 4));
}

TEST(MathTest, VectorInt4_VectorMinMax)
{
    EXPECT_TRUE(VectorInt4::Min(vecB, vecE) == VectorInt4(1, 2, 2, 1));
    EXPECT_TRUE(VectorInt4::Min(vecE, vecB) == VectorInt4(1, 2, 2, 1));
    EXPECT_TRUE(VectorInt4::Max(vecB, vecE) == VectorInt4(4, 3, 3, 4));
    EXPECT_TRUE(VectorInt4::Max(vecB, vecE) == VectorInt4(4, 3, 3, 4));
}

//////////////////////////////////////////////////////////////////////////

TEST(MathTest, VectorInt4_VectorEqual)
{
    EXPECT_TRUE(VectorInt4(1, 2, 3, 4) == VectorInt4(1, 2, 3, 4));
    EXPECT_FALSE(VectorInt4(10, 2, 3, 4) == VectorInt4(1, 2, 3, 4));
    EXPECT_FALSE(VectorInt4(1, 20, 3, 4) == VectorInt4(1, 2, 3, 4));
    EXPECT_FALSE(VectorInt4(1, 2, 30, 4) == VectorInt4(1, 2, 3, 4));
    EXPECT_FALSE(VectorInt4(1, 2, 3, 40) == VectorInt4(1, 2, 3, 4));
}

//////////////////////////////////////////////////////////////////////////

TEST(MathTest, VectorInt4_VectorSwizzle)
{
    const VectorInt4 v(0, 1, 2, 3);

    EXPECT_EQ(VectorInt4(0, 1, 2, 3), (v.Swizzle<0, 1, 2, 3>()));
    EXPECT_EQ(VectorInt4(3, 2, 1, 0), (v.Swizzle<3, 2, 1, 0>()));
    EXPECT_EQ(VectorInt4(0, 0, 0, 0), (v.Swizzle<0, 0, 0, 0>()));
    EXPECT_EQ(VectorInt4(1, 1, 1, 1), (v.Swizzle<1, 1, 1, 1>()));
    EXPECT_EQ(VectorInt4(2, 2, 2, 2), (v.Swizzle<2, 2, 2, 2>()));
    EXPECT_EQ(VectorInt4(3, 3, 3, 3), (v.Swizzle<3, 3, 3, 3>()));

    EXPECT_EQ(VectorInt4(1, 0, 0, 0), (v.Swizzle<1, 0, 0, 0>()));
    EXPECT_EQ(VectorInt4(0, 1, 0, 0), (v.Swizzle<0, 1, 0, 0>()));
    EXPECT_EQ(VectorInt4(0, 0, 1, 0), (v.Swizzle<0, 0, 1, 0>()));
    EXPECT_EQ(VectorInt4(0, 0, 0, 1), (v.Swizzle<0, 0, 0, 1>()));

    EXPECT_EQ(VectorInt4(2, 0, 0, 0), (v.Swizzle<2, 0, 0, 0>()));
    EXPECT_EQ(VectorInt4(0, 2, 0, 0), (v.Swizzle<0, 2, 0, 0>()));
    EXPECT_EQ(VectorInt4(0, 0, 2, 0), (v.Swizzle<0, 0, 2, 0>()));
    EXPECT_EQ(VectorInt4(0, 0, 0, 2), (v.Swizzle<0, 0, 0, 2>()));

    EXPECT_EQ(VectorInt4(3, 0, 0, 0), (v.Swizzle<3, 0, 0, 0>()));
    EXPECT_EQ(VectorInt4(0, 3, 0, 0), (v.Swizzle<0, 3, 0, 0>()));
    EXPECT_EQ(VectorInt4(0, 0, 3, 0), (v.Swizzle<0, 0, 3, 0>()));
    EXPECT_EQ(VectorInt4(0, 0, 0, 3), (v.Swizzle<0, 0, 0, 3>()));
}

//////////////////////////////////////////////////////////////////////////

TEST(MathTest, VectorInt4_SetIfEqual)
{
    const VectorInt4 v(1, 2, 3, 4);

    EXPECT_EQ(VectorInt4(~0, 2, 3, 4), v.SetIfGreaterOrEqual(VectorInt4(1, 9, 9, 9), VectorInt4(~0, ~0, ~0, ~0)));
    EXPECT_EQ(VectorInt4(1, ~0, 3, 4), v.SetIfGreaterOrEqual(VectorInt4(9, 2, 9, 9), VectorInt4(~0, ~0, ~0, ~0)));
    EXPECT_EQ(VectorInt4(1, 2, ~0, 4), v.SetIfGreaterOrEqual(VectorInt4(9, 9, 3, 9), VectorInt4(~0, ~0, ~0, ~0)));
    EXPECT_EQ(VectorInt4(1, 2, 3, ~0), v.SetIfGreaterOrEqual(VectorInt4(9, 9, 9, 4), VectorInt4(~0, ~0, ~0, ~0)));

    EXPECT_EQ(VectorInt4(~0, 2, 3, 4), v.SetIfGreaterOrEqual(VectorInt4(0, 9, 9, 9), VectorInt4(~0, ~0, ~0, ~0)));
    EXPECT_EQ(VectorInt4(1, ~0, 3, 4), v.SetIfGreaterOrEqual(VectorInt4(9, 1, 9, 9), VectorInt4(~0, ~0, ~0, ~0)));
    EXPECT_EQ(VectorInt4(1, 2, ~0, 4), v.SetIfGreaterOrEqual(VectorInt4(9, 9, 2, 9), VectorInt4(~0, ~0, ~0, ~0)));
    EXPECT_EQ(VectorInt4(1, 2, 3, ~0), v.SetIfGreaterOrEqual(VectorInt4(9, 9, 9, 3), VectorInt4(~0, ~0, ~0, ~0)));

    EXPECT_EQ(v, v.SetIfGreaterOrEqual(VectorInt4(2, 3, 4, 5), VectorInt4(~0, ~0, ~0, ~0)));
}
