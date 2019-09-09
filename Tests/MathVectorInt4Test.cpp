#include "PCH.h"
#include "../Core/Math/VectorInt4.h"

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
    EXPECT_TRUE((vecC == (vecA + vecB)).All());
    EXPECT_TRUE((VectorInt4(0, -1, -2, -3) == (vecA - vecB)).All());
    EXPECT_TRUE((vecD == (vecB * vecB)).All());
    EXPECT_TRUE((VectorInt4(2, 4, 6, 8) == (vecB * 2)).All());
    EXPECT_TRUE((VectorInt4(-1, -2, -3, -4) == -VectorInt4(1, 2, 3, 4)).All());
}

TEST(MathTest, VectorInt4_ShiftLeftSharedCount)
{
    for (Uint32 i = 0; i < 32; ++i)
    {
        SCOPED_TRACE("i=" + std::to_string(i));

        VectorInt4 v(0, 1, 123, INT32_MAX);
        const VectorInt4 expected(0, 1 << i, 123 << i, INT32_MAX << i);

        EXPECT_TRUE((expected == (v << i)).All());
        v <<= i;
        EXPECT_TRUE((expected == v).All());
    }
}

TEST(MathTest, VectorInt4_ShiftRightSharedCount)
{
    for (Uint32 i = 0; i < 32; ++i)
    {
        SCOPED_TRACE("i=" + std::to_string(i));

        VectorInt4 v(0, 1, 123456789, INT32_MAX);
        const VectorInt4 expected(0, 1 >> i, 123456789 >> i, INT32_MAX >> i);

        EXPECT_TRUE((expected == (v >> i)).All());
        v >>= i;
        EXPECT_TRUE((expected == v).All());
    }
}

TEST(MathTest, VectorInt4_ShiftLeftGeneric)
{
    VectorInt4 v(1, 1, 1, 1);
    const VectorInt4 count(0, 1, 2, 3);
    const VectorInt4 expected(1 << 0, 1 << 1, 1 << 2, 1 << 3);

    EXPECT_TRUE((expected == (v << count)).All());
    v <<= count;
    EXPECT_TRUE((expected == v).All());
}

TEST(MathTest, VectorInt4_VectorMinMax)
{
    EXPECT_TRUE((VectorInt4::Min(vecB, vecE) == VectorInt4(1, 2, 2, 1)).All());
    EXPECT_TRUE((VectorInt4::Min(vecE, vecB) == VectorInt4(1, 2, 2, 1)).All());
    EXPECT_TRUE((VectorInt4::Max(vecB, vecE) == VectorInt4(4, 3, 3, 4)).All());
    EXPECT_TRUE((VectorInt4::Max(vecB, vecE) == VectorInt4(4, 3, 3, 4)).All());
}

//////////////////////////////////////////////////////////////////////////

TEST(MathTest, VectorInt4_VectorLess)
{
    EXPECT_EQ(VectorBool4(0, 0, 0, 0), VectorInt4(2, 3, 4, 5) < VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(1, 1, 1, 1), VectorInt4(1, 2, 3, 4) < VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(0, 1, 1, 1), VectorInt4(9, 2, 3, 4) < VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(1, 0, 1, 1), VectorInt4(1, 9, 3, 4) < VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(1, 1, 0, 1), VectorInt4(1, 2, 9, 4) < VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(1, 1, 1, 0), VectorInt4(1, 2, 3, 9) < VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(0, 1, 1, 1), VectorInt4(2, 2, 3, 4) < VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(1, 0, 1, 1), VectorInt4(1, 3, 3, 4) < VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(1, 1, 0, 1), VectorInt4(1, 2, 4, 4) < VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(1, 1, 1, 0), VectorInt4(1, 2, 3, 5) < VectorInt4(2, 3, 4, 5));
}

TEST(MathTest, VectorInt4_VectorLessOrEqual)
{
    EXPECT_EQ(VectorBool4(1, 1, 1, 1), VectorInt4(2, 3, 4, 5) <= VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(1, 1, 1, 1), VectorInt4(1, 2, 3, 4) <= VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(0, 1, 1, 1), VectorInt4(9, 2, 3, 4) <= VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(1, 0, 1, 1), VectorInt4(1, 9, 3, 4) <= VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(1, 1, 0, 1), VectorInt4(1, 2, 9, 4) <= VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(1, 1, 1, 0), VectorInt4(1, 2, 3, 9) <= VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(1, 1, 1, 1), VectorInt4(2, 2, 3, 4) <= VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(1, 1, 1, 1), VectorInt4(1, 3, 3, 4) <= VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(1, 1, 1, 1), VectorInt4(1, 2, 4, 4) <= VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(1, 1, 1, 1), VectorInt4(1, 2, 3, 5) <= VectorInt4(2, 3, 4, 5));
}

TEST(MathTest, VectorInt4_VectorGreater)
{
    EXPECT_EQ(VectorBool4(0, 0, 0, 0), VectorInt4(2, 3, 4, 5) > VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(1, 1, 1, 1), VectorInt4(3, 4, 5, 6) > VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(0, 1, 1, 1), VectorInt4(0, 4, 5, 6) > VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(1, 0, 1, 1), VectorInt4(3, 0, 5, 6) > VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(1, 1, 0, 1), VectorInt4(3, 4, 0, 6) > VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(1, 1, 1, 0), VectorInt4(3, 4, 5, 0) > VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(0, 1, 1, 1), VectorInt4(2, 4, 5, 6) > VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(1, 0, 1, 1), VectorInt4(3, 3, 5, 6) > VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(1, 1, 0, 1), VectorInt4(3, 4, 4, 6) > VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(1, 1, 1, 0), VectorInt4(3, 4, 5, 5) > VectorInt4(2, 3, 4, 5));
}

TEST(MathTest, VectorInt4_VectorGreaterOrEqual)
{
    EXPECT_EQ(VectorBool4(1, 1, 1, 1), VectorInt4(2, 3, 4, 5) >= VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(1, 1, 1, 1), VectorInt4(3, 4, 5, 6) >= VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(0, 1, 1, 1), VectorInt4(0, 4, 5, 6) >= VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(1, 0, 1, 1), VectorInt4(3, 0, 5, 6) >= VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(1, 1, 0, 1), VectorInt4(3, 4, 0, 6) >= VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(1, 1, 1, 0), VectorInt4(3, 4, 5, 0) >= VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(1, 1, 1, 1), VectorInt4(2, 4, 5, 6) >= VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(1, 1, 1, 1), VectorInt4(3, 3, 5, 6) >= VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(1, 1, 1, 1), VectorInt4(3, 4, 4, 6) >= VectorInt4(2, 3, 4, 5));
    EXPECT_EQ(VectorBool4(1, 1, 1, 1), VectorInt4(3, 4, 5, 5) >= VectorInt4(2, 3, 4, 5));
}

TEST(MathTest, VectorInt4_VectorEqual)
{
    EXPECT_EQ(VectorBool4(1, 1, 1, 1), VectorInt4(1, 2, 3, 4) == VectorInt4(1, 2, 3, 4));
    EXPECT_EQ(VectorBool4(0, 1, 1, 1), VectorInt4(9, 2, 3, 4) == VectorInt4(1, 2, 3, 4));
    EXPECT_EQ(VectorBool4(1, 0, 1, 1), VectorInt4(1, 9, 3, 4) == VectorInt4(1, 2, 3, 4));
    EXPECT_EQ(VectorBool4(1, 1, 0, 1), VectorInt4(1, 2, 9, 4) == VectorInt4(1, 2, 3, 4));
    EXPECT_EQ(VectorBool4(1, 1, 1, 0), VectorInt4(1, 2, 3, 9) == VectorInt4(1, 2, 3, 4));
}

TEST(MathTest, VectorInt4_VectorNotEqual)
{
    EXPECT_EQ(VectorBool4(1, 1, 1, 1), VectorInt4(4, 3, 2, 1) != VectorInt4(1, 2, 3, 4));
    EXPECT_EQ(VectorBool4(0, 1, 1, 1), VectorInt4(1, 3, 2, 1) != VectorInt4(1, 2, 3, 4));
    EXPECT_EQ(VectorBool4(1, 0, 1, 1), VectorInt4(4, 2, 2, 1) != VectorInt4(1, 2, 3, 4));
    EXPECT_EQ(VectorBool4(1, 1, 0, 1), VectorInt4(4, 3, 3, 1) != VectorInt4(1, 2, 3, 4));
    EXPECT_EQ(VectorBool4(1, 1, 1, 0), VectorInt4(4, 3, 2, 4) != VectorInt4(1, 2, 3, 4));
}

//////////////////////////////////////////////////////////////////////////

TEST(MathTest, VectorInt4_VectorSwizzle)
{
    const VectorInt4 v(0, 1, 2, 3);

    {
        EXPECT_TRUE((VectorInt4(0, 0, 0, 0) == (v.Swizzle<0, 0, 0, 0>())).All());
        EXPECT_TRUE((VectorInt4(0, 0, 0, 1) == (v.Swizzle<0, 0, 0, 1>())).All());
        EXPECT_TRUE((VectorInt4(0, 0, 0, 2) == (v.Swizzle<0, 0, 0, 2>())).All());
        EXPECT_TRUE((VectorInt4(0, 0, 0, 3) == (v.Swizzle<0, 0, 0, 3>())).All());
        EXPECT_TRUE((VectorInt4(0, 0, 1, 0) == (v.Swizzle<0, 0, 1, 0>())).All());
        EXPECT_TRUE((VectorInt4(0, 0, 1, 1) == (v.Swizzle<0, 0, 1, 1>())).All());
        EXPECT_TRUE((VectorInt4(0, 0, 1, 2) == (v.Swizzle<0, 0, 1, 2>())).All());
        EXPECT_TRUE((VectorInt4(0, 0, 1, 3) == (v.Swizzle<0, 0, 1, 3>())).All());
        EXPECT_TRUE((VectorInt4(0, 0, 2, 0) == (v.Swizzle<0, 0, 2, 0>())).All());
        EXPECT_TRUE((VectorInt4(0, 0, 2, 1) == (v.Swizzle<0, 0, 2, 1>())).All());
        EXPECT_TRUE((VectorInt4(0, 0, 2, 2) == (v.Swizzle<0, 0, 2, 2>())).All());
        EXPECT_TRUE((VectorInt4(0, 0, 2, 3) == (v.Swizzle<0, 0, 2, 3>())).All());
        EXPECT_TRUE((VectorInt4(0, 0, 3, 0) == (v.Swizzle<0, 0, 3, 0>())).All());
        EXPECT_TRUE((VectorInt4(0, 0, 3, 1) == (v.Swizzle<0, 0, 3, 1>())).All());
        EXPECT_TRUE((VectorInt4(0, 0, 3, 2) == (v.Swizzle<0, 0, 3, 2>())).All());
        EXPECT_TRUE((VectorInt4(0, 0, 3, 3) == (v.Swizzle<0, 0, 3, 3>())).All());

        EXPECT_TRUE((VectorInt4(0, 1, 0, 0) == (v.Swizzle<0, 1, 0, 0>())).All());
        EXPECT_TRUE((VectorInt4(0, 1, 0, 1) == (v.Swizzle<0, 1, 0, 1>())).All());
        EXPECT_TRUE((VectorInt4(0, 1, 0, 2) == (v.Swizzle<0, 1, 0, 2>())).All());
        EXPECT_TRUE((VectorInt4(0, 1, 0, 3) == (v.Swizzle<0, 1, 0, 3>())).All());
        EXPECT_TRUE((VectorInt4(0, 1, 1, 0) == (v.Swizzle<0, 1, 1, 0>())).All());
        EXPECT_TRUE((VectorInt4(0, 1, 1, 1) == (v.Swizzle<0, 1, 1, 1>())).All());
        EXPECT_TRUE((VectorInt4(0, 1, 1, 2) == (v.Swizzle<0, 1, 1, 2>())).All());
        EXPECT_TRUE((VectorInt4(0, 1, 1, 3) == (v.Swizzle<0, 1, 1, 3>())).All());
        EXPECT_TRUE((VectorInt4(0, 1, 2, 0) == (v.Swizzle<0, 1, 2, 0>())).All());
        EXPECT_TRUE((VectorInt4(0, 1, 2, 1) == (v.Swizzle<0, 1, 2, 1>())).All());
        EXPECT_TRUE((VectorInt4(0, 1, 2, 2) == (v.Swizzle<0, 1, 2, 2>())).All());
        EXPECT_TRUE((VectorInt4(0, 1, 2, 3) == (v.Swizzle<0, 1, 2, 3>())).All());
        EXPECT_TRUE((VectorInt4(0, 1, 3, 0) == (v.Swizzle<0, 1, 3, 0>())).All());
        EXPECT_TRUE((VectorInt4(0, 1, 3, 1) == (v.Swizzle<0, 1, 3, 1>())).All());
        EXPECT_TRUE((VectorInt4(0, 1, 3, 2) == (v.Swizzle<0, 1, 3, 2>())).All());
        EXPECT_TRUE((VectorInt4(0, 1, 3, 3) == (v.Swizzle<0, 1, 3, 3>())).All());

        EXPECT_TRUE((VectorInt4(0, 2, 0, 0) == (v.Swizzle<0, 2, 0, 0>())).All());
        EXPECT_TRUE((VectorInt4(0, 2, 0, 1) == (v.Swizzle<0, 2, 0, 1>())).All());
        EXPECT_TRUE((VectorInt4(0, 2, 0, 2) == (v.Swizzle<0, 2, 0, 2>())).All());
        EXPECT_TRUE((VectorInt4(0, 2, 0, 3) == (v.Swizzle<0, 2, 0, 3>())).All());
        EXPECT_TRUE((VectorInt4(0, 2, 1, 0) == (v.Swizzle<0, 2, 1, 0>())).All());
        EXPECT_TRUE((VectorInt4(0, 2, 1, 1) == (v.Swizzle<0, 2, 1, 1>())).All());
        EXPECT_TRUE((VectorInt4(0, 2, 1, 2) == (v.Swizzle<0, 2, 1, 2>())).All());
        EXPECT_TRUE((VectorInt4(0, 2, 1, 3) == (v.Swizzle<0, 2, 1, 3>())).All());
        EXPECT_TRUE((VectorInt4(0, 2, 2, 0) == (v.Swizzle<0, 2, 2, 0>())).All());
        EXPECT_TRUE((VectorInt4(0, 2, 2, 1) == (v.Swizzle<0, 2, 2, 1>())).All());
        EXPECT_TRUE((VectorInt4(0, 2, 2, 2) == (v.Swizzle<0, 2, 2, 2>())).All());
        EXPECT_TRUE((VectorInt4(0, 2, 2, 3) == (v.Swizzle<0, 2, 2, 3>())).All());
        EXPECT_TRUE((VectorInt4(0, 2, 3, 0) == (v.Swizzle<0, 2, 3, 0>())).All());
        EXPECT_TRUE((VectorInt4(0, 2, 3, 1) == (v.Swizzle<0, 2, 3, 1>())).All());
        EXPECT_TRUE((VectorInt4(0, 2, 3, 2) == (v.Swizzle<0, 2, 3, 2>())).All());
        EXPECT_TRUE((VectorInt4(0, 2, 3, 3) == (v.Swizzle<0, 2, 3, 3>())).All());

        EXPECT_TRUE((VectorInt4(0, 3, 0, 0) == (v.Swizzle<0, 3, 0, 0>())).All());
        EXPECT_TRUE((VectorInt4(0, 3, 0, 1) == (v.Swizzle<0, 3, 0, 1>())).All());
        EXPECT_TRUE((VectorInt4(0, 3, 0, 2) == (v.Swizzle<0, 3, 0, 2>())).All());
        EXPECT_TRUE((VectorInt4(0, 3, 0, 3) == (v.Swizzle<0, 3, 0, 3>())).All());
        EXPECT_TRUE((VectorInt4(0, 3, 1, 0) == (v.Swizzle<0, 3, 1, 0>())).All());
        EXPECT_TRUE((VectorInt4(0, 3, 1, 1) == (v.Swizzle<0, 3, 1, 1>())).All());
        EXPECT_TRUE((VectorInt4(0, 3, 1, 2) == (v.Swizzle<0, 3, 1, 2>())).All());
        EXPECT_TRUE((VectorInt4(0, 3, 1, 3) == (v.Swizzle<0, 3, 1, 3>())).All());
        EXPECT_TRUE((VectorInt4(0, 3, 2, 0) == (v.Swizzle<0, 3, 2, 0>())).All());
        EXPECT_TRUE((VectorInt4(0, 3, 2, 1) == (v.Swizzle<0, 3, 2, 1>())).All());
        EXPECT_TRUE((VectorInt4(0, 3, 2, 2) == (v.Swizzle<0, 3, 2, 2>())).All());
        EXPECT_TRUE((VectorInt4(0, 3, 2, 3) == (v.Swizzle<0, 3, 2, 3>())).All());
        EXPECT_TRUE((VectorInt4(0, 3, 3, 0) == (v.Swizzle<0, 3, 3, 0>())).All());
        EXPECT_TRUE((VectorInt4(0, 3, 3, 1) == (v.Swizzle<0, 3, 3, 1>())).All());
        EXPECT_TRUE((VectorInt4(0, 3, 3, 2) == (v.Swizzle<0, 3, 3, 2>())).All());
        EXPECT_TRUE((VectorInt4(0, 3, 3, 3) == (v.Swizzle<0, 3, 3, 3>())).All());
    }

    {
        EXPECT_TRUE((VectorInt4(1, 0, 0, 0) == (v.Swizzle<1, 0, 0, 0>())).All());
        EXPECT_TRUE((VectorInt4(1, 0, 0, 1) == (v.Swizzle<1, 0, 0, 1>())).All());
        EXPECT_TRUE((VectorInt4(1, 0, 0, 2) == (v.Swizzle<1, 0, 0, 2>())).All());
        EXPECT_TRUE((VectorInt4(1, 0, 0, 3) == (v.Swizzle<1, 0, 0, 3>())).All());
        EXPECT_TRUE((VectorInt4(1, 0, 1, 0) == (v.Swizzle<1, 0, 1, 0>())).All());
        EXPECT_TRUE((VectorInt4(1, 0, 1, 1) == (v.Swizzle<1, 0, 1, 1>())).All());
        EXPECT_TRUE((VectorInt4(1, 0, 1, 2) == (v.Swizzle<1, 0, 1, 2>())).All());
        EXPECT_TRUE((VectorInt4(1, 0, 1, 3) == (v.Swizzle<1, 0, 1, 3>())).All());
        EXPECT_TRUE((VectorInt4(1, 0, 2, 0) == (v.Swizzle<1, 0, 2, 0>())).All());
        EXPECT_TRUE((VectorInt4(1, 0, 2, 1) == (v.Swizzle<1, 0, 2, 1>())).All());
        EXPECT_TRUE((VectorInt4(1, 0, 2, 2) == (v.Swizzle<1, 0, 2, 2>())).All());
        EXPECT_TRUE((VectorInt4(1, 0, 2, 3) == (v.Swizzle<1, 0, 2, 3>())).All());
        EXPECT_TRUE((VectorInt4(1, 0, 3, 0) == (v.Swizzle<1, 0, 3, 0>())).All());
        EXPECT_TRUE((VectorInt4(1, 0, 3, 1) == (v.Swizzle<1, 0, 3, 1>())).All());
        EXPECT_TRUE((VectorInt4(1, 0, 3, 2) == (v.Swizzle<1, 0, 3, 2>())).All());
        EXPECT_TRUE((VectorInt4(1, 0, 3, 3) == (v.Swizzle<1, 0, 3, 3>())).All());

        EXPECT_TRUE((VectorInt4(1, 1, 0, 0) == (v.Swizzle<1, 1, 0, 0>())).All());
        EXPECT_TRUE((VectorInt4(1, 1, 0, 1) == (v.Swizzle<1, 1, 0, 1>())).All());
        EXPECT_TRUE((VectorInt4(1, 1, 0, 2) == (v.Swizzle<1, 1, 0, 2>())).All());
        EXPECT_TRUE((VectorInt4(1, 1, 0, 3) == (v.Swizzle<1, 1, 0, 3>())).All());
        EXPECT_TRUE((VectorInt4(1, 1, 1, 0) == (v.Swizzle<1, 1, 1, 0>())).All());
        EXPECT_TRUE((VectorInt4(1, 1, 1, 1) == (v.Swizzle<1, 1, 1, 1>())).All());
        EXPECT_TRUE((VectorInt4(1, 1, 1, 2) == (v.Swizzle<1, 1, 1, 2>())).All());
        EXPECT_TRUE((VectorInt4(1, 1, 1, 3) == (v.Swizzle<1, 1, 1, 3>())).All());
        EXPECT_TRUE((VectorInt4(1, 1, 2, 0) == (v.Swizzle<1, 1, 2, 0>())).All());
        EXPECT_TRUE((VectorInt4(1, 1, 2, 1) == (v.Swizzle<1, 1, 2, 1>())).All());
        EXPECT_TRUE((VectorInt4(1, 1, 2, 2) == (v.Swizzle<1, 1, 2, 2>())).All());
        EXPECT_TRUE((VectorInt4(1, 1, 2, 3) == (v.Swizzle<1, 1, 2, 3>())).All());
        EXPECT_TRUE((VectorInt4(1, 1, 3, 0) == (v.Swizzle<1, 1, 3, 0>())).All());
        EXPECT_TRUE((VectorInt4(1, 1, 3, 1) == (v.Swizzle<1, 1, 3, 1>())).All());
        EXPECT_TRUE((VectorInt4(1, 1, 3, 2) == (v.Swizzle<1, 1, 3, 2>())).All());
        EXPECT_TRUE((VectorInt4(1, 1, 3, 3) == (v.Swizzle<1, 1, 3, 3>())).All());

        EXPECT_TRUE((VectorInt4(1, 2, 0, 0) == (v.Swizzle<1, 2, 0, 0>())).All());
        EXPECT_TRUE((VectorInt4(1, 2, 0, 1) == (v.Swizzle<1, 2, 0, 1>())).All());
        EXPECT_TRUE((VectorInt4(1, 2, 0, 2) == (v.Swizzle<1, 2, 0, 2>())).All());
        EXPECT_TRUE((VectorInt4(1, 2, 0, 3) == (v.Swizzle<1, 2, 0, 3>())).All());
        EXPECT_TRUE((VectorInt4(1, 2, 1, 0) == (v.Swizzle<1, 2, 1, 0>())).All());
        EXPECT_TRUE((VectorInt4(1, 2, 1, 1) == (v.Swizzle<1, 2, 1, 1>())).All());
        EXPECT_TRUE((VectorInt4(1, 2, 1, 2) == (v.Swizzle<1, 2, 1, 2>())).All());
        EXPECT_TRUE((VectorInt4(1, 2, 1, 3) == (v.Swizzle<1, 2, 1, 3>())).All());
        EXPECT_TRUE((VectorInt4(1, 2, 2, 0) == (v.Swizzle<1, 2, 2, 0>())).All());
        EXPECT_TRUE((VectorInt4(1, 2, 2, 1) == (v.Swizzle<1, 2, 2, 1>())).All());
        EXPECT_TRUE((VectorInt4(1, 2, 2, 2) == (v.Swizzle<1, 2, 2, 2>())).All());
        EXPECT_TRUE((VectorInt4(1, 2, 2, 3) == (v.Swizzle<1, 2, 2, 3>())).All());
        EXPECT_TRUE((VectorInt4(1, 2, 3, 0) == (v.Swizzle<1, 2, 3, 0>())).All());
        EXPECT_TRUE((VectorInt4(1, 2, 3, 1) == (v.Swizzle<1, 2, 3, 1>())).All());
        EXPECT_TRUE((VectorInt4(1, 2, 3, 2) == (v.Swizzle<1, 2, 3, 2>())).All());
        EXPECT_TRUE((VectorInt4(1, 2, 3, 3) == (v.Swizzle<1, 2, 3, 3>())).All());

        EXPECT_TRUE((VectorInt4(1, 3, 0, 0) == (v.Swizzle<1, 3, 0, 0>())).All());
        EXPECT_TRUE((VectorInt4(1, 3, 0, 1) == (v.Swizzle<1, 3, 0, 1>())).All());
        EXPECT_TRUE((VectorInt4(1, 3, 0, 2) == (v.Swizzle<1, 3, 0, 2>())).All());
        EXPECT_TRUE((VectorInt4(1, 3, 0, 3) == (v.Swizzle<1, 3, 0, 3>())).All());
        EXPECT_TRUE((VectorInt4(1, 3, 1, 0) == (v.Swizzle<1, 3, 1, 0>())).All());
        EXPECT_TRUE((VectorInt4(1, 3, 1, 1) == (v.Swizzle<1, 3, 1, 1>())).All());
        EXPECT_TRUE((VectorInt4(1, 3, 1, 2) == (v.Swizzle<1, 3, 1, 2>())).All());
        EXPECT_TRUE((VectorInt4(1, 3, 1, 3) == (v.Swizzle<1, 3, 1, 3>())).All());
        EXPECT_TRUE((VectorInt4(1, 3, 2, 0) == (v.Swizzle<1, 3, 2, 0>())).All());
        EXPECT_TRUE((VectorInt4(1, 3, 2, 1) == (v.Swizzle<1, 3, 2, 1>())).All());
        EXPECT_TRUE((VectorInt4(1, 3, 2, 2) == (v.Swizzle<1, 3, 2, 2>())).All());
        EXPECT_TRUE((VectorInt4(1, 3, 2, 3) == (v.Swizzle<1, 3, 2, 3>())).All());
        EXPECT_TRUE((VectorInt4(1, 3, 3, 0) == (v.Swizzle<1, 3, 3, 0>())).All());
        EXPECT_TRUE((VectorInt4(1, 3, 3, 1) == (v.Swizzle<1, 3, 3, 1>())).All());
        EXPECT_TRUE((VectorInt4(1, 3, 3, 2) == (v.Swizzle<1, 3, 3, 2>())).All());
        EXPECT_TRUE((VectorInt4(1, 3, 3, 3) == (v.Swizzle<1, 3, 3, 3>())).All());
    }

    {
        EXPECT_TRUE((VectorInt4(2, 0, 0, 0) == (v.Swizzle<2, 0, 0, 0>())).All());
        EXPECT_TRUE((VectorInt4(2, 0, 0, 1) == (v.Swizzle<2, 0, 0, 1>())).All());
        EXPECT_TRUE((VectorInt4(2, 0, 0, 2) == (v.Swizzle<2, 0, 0, 2>())).All());
        EXPECT_TRUE((VectorInt4(2, 0, 0, 3) == (v.Swizzle<2, 0, 0, 3>())).All());
        EXPECT_TRUE((VectorInt4(2, 0, 1, 0) == (v.Swizzle<2, 0, 1, 0>())).All());
        EXPECT_TRUE((VectorInt4(2, 0, 1, 1) == (v.Swizzle<2, 0, 1, 1>())).All());
        EXPECT_TRUE((VectorInt4(2, 0, 1, 2) == (v.Swizzle<2, 0, 1, 2>())).All());
        EXPECT_TRUE((VectorInt4(2, 0, 1, 3) == (v.Swizzle<2, 0, 1, 3>())).All());
        EXPECT_TRUE((VectorInt4(2, 0, 2, 0) == (v.Swizzle<2, 0, 2, 0>())).All());
        EXPECT_TRUE((VectorInt4(2, 0, 2, 1) == (v.Swizzle<2, 0, 2, 1>())).All());
        EXPECT_TRUE((VectorInt4(2, 0, 2, 2) == (v.Swizzle<2, 0, 2, 2>())).All());
        EXPECT_TRUE((VectorInt4(2, 0, 2, 3) == (v.Swizzle<2, 0, 2, 3>())).All());
        EXPECT_TRUE((VectorInt4(2, 0, 3, 0) == (v.Swizzle<2, 0, 3, 0>())).All());
        EXPECT_TRUE((VectorInt4(2, 0, 3, 1) == (v.Swizzle<2, 0, 3, 1>())).All());
        EXPECT_TRUE((VectorInt4(2, 0, 3, 2) == (v.Swizzle<2, 0, 3, 2>())).All());
        EXPECT_TRUE((VectorInt4(2, 0, 3, 3) == (v.Swizzle<2, 0, 3, 3>())).All());

        EXPECT_TRUE((VectorInt4(2, 1, 0, 0) == (v.Swizzle<2, 1, 0, 0>())).All());
        EXPECT_TRUE((VectorInt4(2, 1, 0, 1) == (v.Swizzle<2, 1, 0, 1>())).All());
        EXPECT_TRUE((VectorInt4(2, 1, 0, 2) == (v.Swizzle<2, 1, 0, 2>())).All());
        EXPECT_TRUE((VectorInt4(2, 1, 0, 3) == (v.Swizzle<2, 1, 0, 3>())).All());
        EXPECT_TRUE((VectorInt4(2, 1, 1, 0) == (v.Swizzle<2, 1, 1, 0>())).All());
        EXPECT_TRUE((VectorInt4(2, 1, 1, 1) == (v.Swizzle<2, 1, 1, 1>())).All());
        EXPECT_TRUE((VectorInt4(2, 1, 1, 2) == (v.Swizzle<2, 1, 1, 2>())).All());
        EXPECT_TRUE((VectorInt4(2, 1, 1, 3) == (v.Swizzle<2, 1, 1, 3>())).All());
        EXPECT_TRUE((VectorInt4(2, 1, 2, 0) == (v.Swizzle<2, 1, 2, 0>())).All());
        EXPECT_TRUE((VectorInt4(2, 1, 2, 1) == (v.Swizzle<2, 1, 2, 1>())).All());
        EXPECT_TRUE((VectorInt4(2, 1, 2, 2) == (v.Swizzle<2, 1, 2, 2>())).All());
        EXPECT_TRUE((VectorInt4(2, 1, 2, 3) == (v.Swizzle<2, 1, 2, 3>())).All());
        EXPECT_TRUE((VectorInt4(2, 1, 3, 0) == (v.Swizzle<2, 1, 3, 0>())).All());
        EXPECT_TRUE((VectorInt4(2, 1, 3, 1) == (v.Swizzle<2, 1, 3, 1>())).All());
        EXPECT_TRUE((VectorInt4(2, 1, 3, 2) == (v.Swizzle<2, 1, 3, 2>())).All());
        EXPECT_TRUE((VectorInt4(2, 1, 3, 3) == (v.Swizzle<2, 1, 3, 3>())).All());

        EXPECT_TRUE((VectorInt4(2, 2, 0, 0) == (v.Swizzle<2, 2, 0, 0>())).All());
        EXPECT_TRUE((VectorInt4(2, 2, 0, 1) == (v.Swizzle<2, 2, 0, 1>())).All());
        EXPECT_TRUE((VectorInt4(2, 2, 0, 2) == (v.Swizzle<2, 2, 0, 2>())).All());
        EXPECT_TRUE((VectorInt4(2, 2, 0, 3) == (v.Swizzle<2, 2, 0, 3>())).All());
        EXPECT_TRUE((VectorInt4(2, 2, 1, 0) == (v.Swizzle<2, 2, 1, 0>())).All());
        EXPECT_TRUE((VectorInt4(2, 2, 1, 1) == (v.Swizzle<2, 2, 1, 1>())).All());
        EXPECT_TRUE((VectorInt4(2, 2, 1, 2) == (v.Swizzle<2, 2, 1, 2>())).All());
        EXPECT_TRUE((VectorInt4(2, 2, 1, 3) == (v.Swizzle<2, 2, 1, 3>())).All());
        EXPECT_TRUE((VectorInt4(2, 2, 2, 0) == (v.Swizzle<2, 2, 2, 0>())).All());
        EXPECT_TRUE((VectorInt4(2, 2, 2, 1) == (v.Swizzle<2, 2, 2, 1>())).All());
        EXPECT_TRUE((VectorInt4(2, 2, 2, 2) == (v.Swizzle<2, 2, 2, 2>())).All());
        EXPECT_TRUE((VectorInt4(2, 2, 2, 3) == (v.Swizzle<2, 2, 2, 3>())).All());
        EXPECT_TRUE((VectorInt4(2, 2, 3, 0) == (v.Swizzle<2, 2, 3, 0>())).All());
        EXPECT_TRUE((VectorInt4(2, 2, 3, 1) == (v.Swizzle<2, 2, 3, 1>())).All());
        EXPECT_TRUE((VectorInt4(2, 2, 3, 2) == (v.Swizzle<2, 2, 3, 2>())).All());
        EXPECT_TRUE((VectorInt4(2, 2, 3, 3) == (v.Swizzle<2, 2, 3, 3>())).All());

        EXPECT_TRUE((VectorInt4(2, 3, 0, 0) == (v.Swizzle<2, 3, 0, 0>())).All());
        EXPECT_TRUE((VectorInt4(2, 3, 0, 1) == (v.Swizzle<2, 3, 0, 1>())).All());
        EXPECT_TRUE((VectorInt4(2, 3, 0, 2) == (v.Swizzle<2, 3, 0, 2>())).All());
        EXPECT_TRUE((VectorInt4(2, 3, 0, 3) == (v.Swizzle<2, 3, 0, 3>())).All());
        EXPECT_TRUE((VectorInt4(2, 3, 1, 0) == (v.Swizzle<2, 3, 1, 0>())).All());
        EXPECT_TRUE((VectorInt4(2, 3, 1, 1) == (v.Swizzle<2, 3, 1, 1>())).All());
        EXPECT_TRUE((VectorInt4(2, 3, 1, 2) == (v.Swizzle<2, 3, 1, 2>())).All());
        EXPECT_TRUE((VectorInt4(2, 3, 1, 3) == (v.Swizzle<2, 3, 1, 3>())).All());
        EXPECT_TRUE((VectorInt4(2, 3, 2, 0) == (v.Swizzle<2, 3, 2, 0>())).All());
        EXPECT_TRUE((VectorInt4(2, 3, 2, 1) == (v.Swizzle<2, 3, 2, 1>())).All());
        EXPECT_TRUE((VectorInt4(2, 3, 2, 2) == (v.Swizzle<2, 3, 2, 2>())).All());
        EXPECT_TRUE((VectorInt4(2, 3, 2, 3) == (v.Swizzle<2, 3, 2, 3>())).All());
        EXPECT_TRUE((VectorInt4(2, 3, 3, 0) == (v.Swizzle<2, 3, 3, 0>())).All());
        EXPECT_TRUE((VectorInt4(2, 3, 3, 1) == (v.Swizzle<2, 3, 3, 1>())).All());
        EXPECT_TRUE((VectorInt4(2, 3, 3, 2) == (v.Swizzle<2, 3, 3, 2>())).All());
        EXPECT_TRUE((VectorInt4(2, 3, 3, 3) == (v.Swizzle<2, 3, 3, 3>())).All());
    }

    {
        EXPECT_TRUE((VectorInt4(3, 0, 0, 0) == (v.Swizzle<3, 0, 0, 0>())).All());
        EXPECT_TRUE((VectorInt4(3, 0, 0, 1) == (v.Swizzle<3, 0, 0, 1>())).All());
        EXPECT_TRUE((VectorInt4(3, 0, 0, 2) == (v.Swizzle<3, 0, 0, 2>())).All());
        EXPECT_TRUE((VectorInt4(3, 0, 0, 3) == (v.Swizzle<3, 0, 0, 3>())).All());
        EXPECT_TRUE((VectorInt4(3, 0, 1, 0) == (v.Swizzle<3, 0, 1, 0>())).All());
        EXPECT_TRUE((VectorInt4(3, 0, 1, 1) == (v.Swizzle<3, 0, 1, 1>())).All());
        EXPECT_TRUE((VectorInt4(3, 0, 1, 2) == (v.Swizzle<3, 0, 1, 2>())).All());
        EXPECT_TRUE((VectorInt4(3, 0, 1, 3) == (v.Swizzle<3, 0, 1, 3>())).All());
        EXPECT_TRUE((VectorInt4(3, 0, 2, 0) == (v.Swizzle<3, 0, 2, 0>())).All());
        EXPECT_TRUE((VectorInt4(3, 0, 2, 1) == (v.Swizzle<3, 0, 2, 1>())).All());
        EXPECT_TRUE((VectorInt4(3, 0, 2, 2) == (v.Swizzle<3, 0, 2, 2>())).All());
        EXPECT_TRUE((VectorInt4(3, 0, 2, 3) == (v.Swizzle<3, 0, 2, 3>())).All());
        EXPECT_TRUE((VectorInt4(3, 0, 3, 0) == (v.Swizzle<3, 0, 3, 0>())).All());
        EXPECT_TRUE((VectorInt4(3, 0, 3, 1) == (v.Swizzle<3, 0, 3, 1>())).All());
        EXPECT_TRUE((VectorInt4(3, 0, 3, 2) == (v.Swizzle<3, 0, 3, 2>())).All());
        EXPECT_TRUE((VectorInt4(3, 0, 3, 3) == (v.Swizzle<3, 0, 3, 3>())).All());

        EXPECT_TRUE((VectorInt4(3, 1, 0, 0) == (v.Swizzle<3, 1, 0, 0>())).All());
        EXPECT_TRUE((VectorInt4(3, 1, 0, 1) == (v.Swizzle<3, 1, 0, 1>())).All());
        EXPECT_TRUE((VectorInt4(3, 1, 0, 2) == (v.Swizzle<3, 1, 0, 2>())).All());
        EXPECT_TRUE((VectorInt4(3, 1, 0, 3) == (v.Swizzle<3, 1, 0, 3>())).All());
        EXPECT_TRUE((VectorInt4(3, 1, 1, 0) == (v.Swizzle<3, 1, 1, 0>())).All());
        EXPECT_TRUE((VectorInt4(3, 1, 1, 1) == (v.Swizzle<3, 1, 1, 1>())).All());
        EXPECT_TRUE((VectorInt4(3, 1, 1, 2) == (v.Swizzle<3, 1, 1, 2>())).All());
        EXPECT_TRUE((VectorInt4(3, 1, 1, 3) == (v.Swizzle<3, 1, 1, 3>())).All());
        EXPECT_TRUE((VectorInt4(3, 1, 2, 0) == (v.Swizzle<3, 1, 2, 0>())).All());
        EXPECT_TRUE((VectorInt4(3, 1, 2, 1) == (v.Swizzle<3, 1, 2, 1>())).All());
        EXPECT_TRUE((VectorInt4(3, 1, 2, 2) == (v.Swizzle<3, 1, 2, 2>())).All());
        EXPECT_TRUE((VectorInt4(3, 1, 2, 3) == (v.Swizzle<3, 1, 2, 3>())).All());
        EXPECT_TRUE((VectorInt4(3, 1, 3, 0) == (v.Swizzle<3, 1, 3, 0>())).All());
        EXPECT_TRUE((VectorInt4(3, 1, 3, 1) == (v.Swizzle<3, 1, 3, 1>())).All());
        EXPECT_TRUE((VectorInt4(3, 1, 3, 2) == (v.Swizzle<3, 1, 3, 2>())).All());
        EXPECT_TRUE((VectorInt4(3, 1, 3, 3) == (v.Swizzle<3, 1, 3, 3>())).All());

        EXPECT_TRUE((VectorInt4(3, 2, 0, 0) == (v.Swizzle<3, 2, 0, 0>())).All());
        EXPECT_TRUE((VectorInt4(3, 2, 0, 1) == (v.Swizzle<3, 2, 0, 1>())).All());
        EXPECT_TRUE((VectorInt4(3, 2, 0, 2) == (v.Swizzle<3, 2, 0, 2>())).All());
        EXPECT_TRUE((VectorInt4(3, 2, 0, 3) == (v.Swizzle<3, 2, 0, 3>())).All());
        EXPECT_TRUE((VectorInt4(3, 2, 1, 0) == (v.Swizzle<3, 2, 1, 0>())).All());
        EXPECT_TRUE((VectorInt4(3, 2, 1, 1) == (v.Swizzle<3, 2, 1, 1>())).All());
        EXPECT_TRUE((VectorInt4(3, 2, 1, 2) == (v.Swizzle<3, 2, 1, 2>())).All());
        EXPECT_TRUE((VectorInt4(3, 2, 1, 3) == (v.Swizzle<3, 2, 1, 3>())).All());
        EXPECT_TRUE((VectorInt4(3, 2, 2, 0) == (v.Swizzle<3, 2, 2, 0>())).All());
        EXPECT_TRUE((VectorInt4(3, 2, 2, 1) == (v.Swizzle<3, 2, 2, 1>())).All());
        EXPECT_TRUE((VectorInt4(3, 2, 2, 2) == (v.Swizzle<3, 2, 2, 2>())).All());
        EXPECT_TRUE((VectorInt4(3, 2, 2, 3) == (v.Swizzle<3, 2, 2, 3>())).All());
        EXPECT_TRUE((VectorInt4(3, 2, 3, 0) == (v.Swizzle<3, 2, 3, 0>())).All());
        EXPECT_TRUE((VectorInt4(3, 2, 3, 1) == (v.Swizzle<3, 2, 3, 1>())).All());
        EXPECT_TRUE((VectorInt4(3, 2, 3, 2) == (v.Swizzle<3, 2, 3, 2>())).All());
        EXPECT_TRUE((VectorInt4(3, 2, 3, 3) == (v.Swizzle<3, 2, 3, 3>())).All());

        EXPECT_TRUE((VectorInt4(3, 3, 0, 0) == (v.Swizzle<3, 3, 0, 0>())).All());
        EXPECT_TRUE((VectorInt4(3, 3, 0, 1) == (v.Swizzle<3, 3, 0, 1>())).All());
        EXPECT_TRUE((VectorInt4(3, 3, 0, 2) == (v.Swizzle<3, 3, 0, 2>())).All());
        EXPECT_TRUE((VectorInt4(3, 3, 0, 3) == (v.Swizzle<3, 3, 0, 3>())).All());
        EXPECT_TRUE((VectorInt4(3, 3, 1, 0) == (v.Swizzle<3, 3, 1, 0>())).All());
        EXPECT_TRUE((VectorInt4(3, 3, 1, 1) == (v.Swizzle<3, 3, 1, 1>())).All());
        EXPECT_TRUE((VectorInt4(3, 3, 1, 2) == (v.Swizzle<3, 3, 1, 2>())).All());
        EXPECT_TRUE((VectorInt4(3, 3, 1, 3) == (v.Swizzle<3, 3, 1, 3>())).All());
        EXPECT_TRUE((VectorInt4(3, 3, 2, 0) == (v.Swizzle<3, 3, 2, 0>())).All());
        EXPECT_TRUE((VectorInt4(3, 3, 2, 1) == (v.Swizzle<3, 3, 2, 1>())).All());
        EXPECT_TRUE((VectorInt4(3, 3, 2, 2) == (v.Swizzle<3, 3, 2, 2>())).All());
        EXPECT_TRUE((VectorInt4(3, 3, 2, 3) == (v.Swizzle<3, 3, 2, 3>())).All());
        EXPECT_TRUE((VectorInt4(3, 3, 3, 0) == (v.Swizzle<3, 3, 3, 0>())).All());
        EXPECT_TRUE((VectorInt4(3, 3, 3, 1) == (v.Swizzle<3, 3, 3, 1>())).All());
        EXPECT_TRUE((VectorInt4(3, 3, 3, 2) == (v.Swizzle<3, 3, 3, 2>())).All());
        EXPECT_TRUE((VectorInt4(3, 3, 3, 3) == (v.Swizzle<3, 3, 3, 3>())).All());
    }
}
