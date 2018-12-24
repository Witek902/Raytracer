#include "PCH.h"
#include "../Core/Math/Vector4.h"

#include "gtest/gtest.h"

using namespace rt::math;

namespace {

static const Vector4 vecA = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
static const Vector4 vecB = Vector4(1.0f, 2.0f, 3.0f, 4.0f);
static const Vector4 vecC = Vector4(2.0f, 3.0f, 4.0f, 5.0f);
static const Vector4 vecD = Vector4(1.0f, 4.0f, 9.0f, 16.0f);
static const Vector4 vecE = Vector4(4.0f, 3.0f, 2.0f, 1.0f);

} // namespace

TEST(MathTest, Vector4_Constructor1)
{
    const Vector4 v(1.0f, 2.0f, 3.0f, 4.0f);
    EXPECT_EQ(1.0f, v.x);
    EXPECT_EQ(2.0f, v.y);
    EXPECT_EQ(3.0f, v.z);
    EXPECT_EQ(4.0f, v.w);
}

TEST(MathTest, Vector4_Constructor2)
{
    const Vector4 v(7.0f);
    EXPECT_EQ(7.0f, v.x);
    EXPECT_EQ(7.0f, v.y);
    EXPECT_EQ(7.0f, v.z);
    EXPECT_EQ(7.0f, v.w);
}

TEST(MathTest, Vector4_Invalid)
{
    EXPECT_TRUE(Vector4(0.0f).IsValid());
    EXPECT_TRUE(Vector4(-1.0f, 2.0f, 3.0f, 4.0f).IsValid());
    EXPECT_TRUE(Vector4(-1.0f, 2.0f, 3.0f, std::numeric_limits<float>::max()).IsValid());
    EXPECT_TRUE(Vector4(-1.0f, 2.0f, 3.0f, std::numeric_limits<float>::min()).IsValid());
    EXPECT_TRUE(Vector4(-1.0f, 2.0f, 3.0f, -std::numeric_limits<float>::max()).IsValid());
    EXPECT_TRUE(Vector4(-1.0f, 2.0f, 3.0f, -std::numeric_limits<float>::min()).IsValid());

    EXPECT_FALSE(Vector4(-1.0f, std::numeric_limits<float>::quiet_NaN(), 3.0f, 4.0f).IsValid());
    EXPECT_FALSE(Vector4(-1.0f, std::numeric_limits<float>::infinity(), 3.0f, 4.0f).IsValid());
    EXPECT_FALSE(Vector4(-1.0f, -std::numeric_limits<float>::infinity(), 3.0f, 4.0f).IsValid());
}

TEST(MathTest, Vector4_VectorLoadAndStore)
{
    Float2 f2;
    Float3 f3;

    vecB.Store(&f2);
    vecB.Store(&f3);
    EXPECT_TRUE(f2.x == 1.0f && f2.y == 2.0f);
    EXPECT_TRUE(f3.x == 1.0f && f3.y == 2.0f && f3.z == 3.0f);
    EXPECT_TRUE((Vector4(f2) == Vector4(1.0f, 2.0f, 0.0f, 0.0f)).All());
    EXPECT_TRUE((Vector4(f3) == Vector4(1.0f, 2.0f, 3.0f, 0.0f)).All());
    EXPECT_TRUE((Vector4(1.0f, 1.0f, 1.0f, 1.0f) == vecB.SplatX()).All());
    EXPECT_TRUE((Vector4(2.0f, 2.0f, 2.0f, 2.0f) == vecB.SplatY()).All());
    EXPECT_TRUE((Vector4(3.0f, 3.0f, 3.0f, 3.0f) == vecB.SplatZ()).All());
    EXPECT_TRUE((Vector4(4.0f, 4.0f, 4.0f, 4.0f) == vecB.SplatW()).All());
}

TEST(MathTest, Vector4_SelectBySign)
{
    Vector4 vA(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 vB(5.0f, 6.0f, 7.0f, 8.0f);

    EXPECT_TRUE((Vector4(1.0f, 2.0f, 3.0f, 4.0f) == Vector4::SelectBySign(vA, vB, Vector4(1.0f, 1.0f, 1.0f, 1.0f))).All());
    EXPECT_TRUE((Vector4(5.0f, 2.0f, 3.0f, 4.0f) == Vector4::SelectBySign(vA, vB, Vector4(-1.0f, 1.0f, 1.0f, 1.0f))).All());
    EXPECT_TRUE((Vector4(1.0f, 6.0f, 3.0f, 4.0f) == Vector4::SelectBySign(vA, vB, Vector4(1.0f, -1.0f, 1.0f, 1.0f))).All());
    EXPECT_TRUE((Vector4(1.0f, 2.0f, 7.0f, 4.0f) == Vector4::SelectBySign(vA, vB, Vector4(1.0f, 1.0f, -1.0f, 1.0f))).All());
    EXPECT_TRUE((Vector4(1.0f, 2.0f, 3.0f, 8.0f) == Vector4::SelectBySign(vA, vB, Vector4(1.0f, 1.0f, 1.0f, -1.0f))).All());
    EXPECT_TRUE((Vector4(5.0f, 6.0f, 7.0f, 8.0f) == Vector4::SelectBySign(vA, vB, Vector4(-1.0f, -1.0f, -1.0f, -1.0f))).All());
}

//////////////////////////////////////////////////////////////////////////

TEST(MathTest, Vector4_VectorArithmetics)
{
    EXPECT_TRUE(Vector4::AlmostEqual(vecA + vecB, vecC));
    EXPECT_TRUE(Vector4::AlmostEqual(vecA - vecB, Vector4(0.0f, -1.0f, -2.0f, -3.0f)));
    EXPECT_TRUE(Vector4::AlmostEqual(vecB * vecB, vecD));
    EXPECT_TRUE(Vector4::AlmostEqual(vecC / vecB, Vector4(2.0f, 1.5f, 4.0f / 3.0f, 1.25f)));
    EXPECT_TRUE(Vector4::AlmostEqual(vecB * 2.0f, Vector4(2.0f, 4.0f, 6.0f, 8.0f)));
    EXPECT_TRUE(Vector4::AlmostEqual(vecB / 2.0f, Vector4(0.5f, 1.0f, 1.5f, 2.0f)));
    EXPECT_TRUE((Vector4::Abs(Vector4(-1.0f, -2.0f, 0.0f, 3.0f)) == Vector4(1.0f, 2.0f, 0.0f, 3.0f)).All());
}

TEST(MathTest, Vector4_VectorLerp)
{
    EXPECT_TRUE((Vector4::Lerp(vecA, vecB, 0.0f) == vecA).All());
    EXPECT_TRUE((Vector4::Lerp(vecA, vecB, 1.0f) == vecB).All());
    EXPECT_TRUE((Vector4::Lerp(vecA, vecB, 0.5f) == Vector4(1.0f, 1.5f, 2.0f, 2.5f)).All());
}

TEST(MathTest, Vector4_VectorMinMax)
{
    EXPECT_TRUE((Vector4::Min(vecB, vecE) == Vector4(1.0f, 2.0f, 2.0f, 1.0f)).All());
    EXPECT_TRUE((Vector4::Min(vecE, vecB) == Vector4(1.0f, 2.0f, 2.0f, 1.0f)).All());
    EXPECT_TRUE((Vector4::Max(vecB, vecE) == Vector4(4.0f, 3.0f, 3.0f, 4.0f)).All());
    EXPECT_TRUE((Vector4::Max(vecB, vecE) == Vector4(4.0f, 3.0f, 3.0f, 4.0f)).All());
}

TEST(MathTest, Vector4_VectorDotProduct)
{
    EXPECT_EQ(8.0f, Vector4::Dot2(vecB, vecC));
    EXPECT_EQ(20.0f, Vector4::Dot3(vecB, vecC));
    EXPECT_EQ(40.0f, Vector4::Dot4(vecB, vecC));

    EXPECT_TRUE((Vector4::Dot2V(vecB, vecC) == Vector4(8.0f)).All());
    EXPECT_TRUE((Vector4::Dot3V(vecB, vecC) == Vector4(20.0f)).All());
    EXPECT_TRUE((Vector4::Dot4V(vecB, vecC) == Vector4(40.0f)).All());
}

TEST(MathTest, Vector4_VectorCrossProduct)
{
    EXPECT_TRUE((Vector4::Cross3(vecB, vecC) == Vector4(-1.0f, 2.0f, -1.0f, 0.0f)).All());
    EXPECT_TRUE((Vector4::Cross3(vecC, vecB) == Vector4(1.0f, -2.0f, 1.0f, 0.0f)).All());
}

TEST(MathTest, Vector4_VectorNormalized)
{
    EXPECT_TRUE(Vector4::AlmostEqual(Vector4(1.0f, 2.0f, 3.0f, 4.0f).Normalized3() & Vector4::MakeMask<1,1,1,0>(),
                                    Vector4(1.0f / sqrtf(14.0f), 2.0f / sqrtf(14.0f), 3.0f / sqrtf(14.0f), 0.0f)));
    EXPECT_TRUE(Vector4::AlmostEqual(Vector4(1.0f, 2.0f, 3.0f, 4.0f).Normalized4(),
                                    Vector4(1.0f / sqrtf(30.0f), 2.0f / sqrtf(30.0f), 3.0f / sqrtf(30.0f), 4.0f / sqrtf(30.0f))));
}

TEST(MathTest, Vector4_VectorNormalize3)
{
    Vector4 v = Vector4(1.0f, 2.0f, 3.0f, 4.0f);

    v.Normalize3();

    EXPECT_TRUE(Vector4::AlmostEqual(v & Vector4::MakeMask<1, 1, 1, 0>(),
                                    Vector4(1.0f / sqrtf(14.0f), 2.0f / sqrtf(14.0f), 3.0f / sqrtf(14.0f), 0.0f)));
}

TEST(MathTest, Vector4_VectorNormalize4)
{
    Vector4 v = Vector4(1.0f, 2.0f, 3.0f, 4.0f);

    v.Normalize4();

    EXPECT_TRUE(Vector4::AlmostEqual(v,
        Vector4(1.0f / sqrtf(30.0f), 2.0f / sqrtf(30.0f), 3.0f / sqrtf(30.0f), 4.0f / sqrtf(30.0f))));
}

TEST(MathTest, Vector4_FusedMultiplyAndAdd)
{
    const Vector4 a(0.5f, 1.0f, 2.0f, 3.0f);
    const Vector4 b(4.0f, 5.0f, 6.0f, 7.0f);
    const Vector4 c(1.5f, 1.5f, 1.5f, 1.5f);

    EXPECT_TRUE((Vector4(3.5f, 6.5f, 13.5f, 22.5f) == Vector4::MulAndAdd(a, b, c)).All());
    EXPECT_TRUE((Vector4(0.5f, 3.5f, 10.5f, 19.5f) == Vector4::MulAndSub(a, b, c)).All());
    EXPECT_TRUE((Vector4(-0.5f, -3.5f, -10.5f, -19.5f) == Vector4::NegMulAndAdd(a, b, c)).All());
    EXPECT_TRUE((Vector4(-3.5f, -6.5f, -13.5f, -22.5f) == Vector4::NegMulAndSub(a, b, c)).All());
}

//////////////////////////////////////////////////////////////////////////

TEST(MathTest, Vector4_VectorLess)
{
    EXPECT_EQ(VectorBool4(false, false, false, false), Vector4(2.0f, 3.0f, 4.0f, 5.0f) < vecC);
    EXPECT_EQ(VectorBool4(true, true, true, true), Vector4(1.0f, 2.0f, 3.0f, 4.0f) < vecC);
    EXPECT_EQ(VectorBool4(false, true, true, true), Vector4(9.0f, 2.0f, 3.0f, 4.0f) < vecC);
    EXPECT_EQ(VectorBool4(true, false, true, true), Vector4(1.0f, 9.0f, 3.0f, 4.0f) < vecC);
    EXPECT_EQ(VectorBool4(true, true, false, true), Vector4(1.0f, 2.0f, 9.0f, 4.0f) < vecC);
    EXPECT_EQ(VectorBool4(true, true, true, false), Vector4(1.0f, 2.0f, 3.0f, 9.0f) < vecC);
    EXPECT_EQ(VectorBool4(false, true, true, true), Vector4(2.0f, 2.0f, 3.0f, 4.0f) < vecC);
    EXPECT_EQ(VectorBool4(true, false, true, true), Vector4(1.0f, 3.0f, 3.0f, 4.0f) < vecC);
    EXPECT_EQ(VectorBool4(true, true, false, true), Vector4(1.0f, 2.0f, 4.0f, 4.0f) < vecC);
    EXPECT_EQ(VectorBool4(true, true, true, false), Vector4(1.0f, 2.0f, 3.0f, 5.0f) < vecC);
}

TEST(MathTest, Vector4_VectorLessOrEqual)
{
    EXPECT_EQ(VectorBool4(true, true, true, true), Vector4(2.0f, 3.0f, 4.0f, 5.0f) <= vecC);
    EXPECT_EQ(VectorBool4(true, true, true, true), Vector4(1.0f, 2.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_EQ(VectorBool4(false, true, true, true), Vector4(9.0f, 2.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_EQ(VectorBool4(true, false, true, true), Vector4(1.0f, 9.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_EQ(VectorBool4(true, true, false, true), Vector4(1.0f, 2.0f, 9.0f, 4.0f) <= vecC);
    EXPECT_EQ(VectorBool4(true, true, true, false), Vector4(1.0f, 2.0f, 3.0f, 9.0f) <= vecC);
    EXPECT_EQ(VectorBool4(true, true, true, true), Vector4(2.0f, 2.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_EQ(VectorBool4(true, true, true, true), Vector4(1.0f, 3.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_EQ(VectorBool4(true, true, true, true), Vector4(1.0f, 2.0f, 4.0f, 4.0f) <= vecC);
    EXPECT_EQ(VectorBool4(true, true, true, true), Vector4(1.0f, 2.0f, 3.0f, 5.0f) <= vecC);
}

TEST(MathTest, Vector4_VectorGreater)
{
    EXPECT_EQ(VectorBool4(false, false, false, false), Vector4(2.0f, 3.0f, 4.0f, 5.0f) > vecC);
    EXPECT_EQ(VectorBool4(true, true, true, true), Vector4(3.0f, 4.0f, 5.0f, 6.0f) > vecC);
    EXPECT_EQ(VectorBool4(false, true, true, true), Vector4(0.0f, 4.0f, 5.0f, 6.0f) > vecC);
    EXPECT_EQ(VectorBool4(true, false, true, true), Vector4(3.0f, 0.0f, 5.0f, 6.0f) > vecC);
    EXPECT_EQ(VectorBool4(true, true, false, true), Vector4(3.0f, 4.0f, 0.0f, 6.0f) > vecC);
    EXPECT_EQ(VectorBool4(true, true, true, false), Vector4(3.0f, 4.0f, 5.0f, 0.0f) > vecC);
    EXPECT_EQ(VectorBool4(false, true, true, true), Vector4(2.0f, 4.0f, 5.0f, 6.0f) > vecC);
    EXPECT_EQ(VectorBool4(true, false, true, true), Vector4(3.0f, 3.0f, 5.0f, 6.0f) > vecC);
    EXPECT_EQ(VectorBool4(true, true, false, true), Vector4(3.0f, 4.0f, 4.0f, 6.0f) > vecC);
    EXPECT_EQ(VectorBool4(true, true, true, false), Vector4(3.0f, 4.0f, 5.0f, 5.0f) > vecC);
}

TEST(MathTest, Vector4_VectorGreaterOrEqual)
{
    EXPECT_EQ(VectorBool4(true, true, true, true), Vector4(2.0f, 3.0f, 4.0f, 5.0f) >= vecC);
    EXPECT_EQ(VectorBool4(true, true, true, true), Vector4(3.0f, 4.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_EQ(VectorBool4(false, true, true, true), Vector4(0.0f, 4.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_EQ(VectorBool4(true, false, true, true), Vector4(3.0f, 0.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_EQ(VectorBool4(true, true, false, true), Vector4(3.0f, 4.0f, 0.0f, 6.0f) >= vecC);
    EXPECT_EQ(VectorBool4(true, true, true, false), Vector4(3.0f, 4.0f, 5.0f, 0.0f) >= vecC);
    EXPECT_EQ(VectorBool4(true, true, true, true), Vector4(2.0f, 4.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_EQ(VectorBool4(true, true, true, true), Vector4(3.0f, 3.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_EQ(VectorBool4(true, true, true, true), Vector4(3.0f, 4.0f, 4.0f, 6.0f) >= vecC);
    EXPECT_EQ(VectorBool4(true, true, true, true), Vector4(3.0f, 4.0f, 5.0f, 5.0f) >= vecC);
}

TEST(MathTest, Vector4_VectorEqual)
{
    EXPECT_EQ(VectorBool4(true, true, true, true), Vector4(1.0f, 2.0f, 3.0f, 4.0f) == Vector4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_EQ(VectorBool4(false, true, true, true), Vector4(10.0f, 2.0f, 3.0f, 4.0f) == Vector4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_EQ(VectorBool4(true, false, true, true), Vector4(1.0f, 20.0f, 3.0f, 4.0f) == Vector4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_EQ(VectorBool4(true, true, false, true), Vector4(1.0f, 2.0f, 30.0f, 4.0f) == Vector4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_EQ(VectorBool4(true, true, true, false), Vector4(1.0f, 2.0f, 3.0f, 40.0f) == Vector4(1.0f, 2.0f, 3.0f, 4.0f));
}

TEST(MathTest, Vector4_VectorNotEqual)
{
    EXPECT_EQ(VectorBool4(true, true, true, true), Vector4(4.0f, 3.0f, 2.0f, 1.0f) != Vector4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_EQ(VectorBool4(false, true, true, true), Vector4(1.0f, 3.0f, 2.0f, 1.0f) != Vector4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_EQ(VectorBool4(true, false, true, true), Vector4(4.0f, 2.0f, 2.0f, 1.0f) != Vector4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_EQ(VectorBool4(true, true, false, true), Vector4(4.0f, 3.0f, 3.0f, 1.0f) != Vector4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_EQ(VectorBool4(true, true, true, false), Vector4(4.0f, 3.0f, 2.0f, 4.0f) != Vector4(1.0f, 2.0f, 3.0f, 4.0f));
}

//////////////////////////////////////////////////////////////////////////

TEST(MathTest, Vector4_VectorSwizzle)
{
    const Vector4 v(0.0f, 1.0f, 2.0f, 3.0f);

    EXPECT_TRUE((Vector4(0.0f, 1.0f, 2.0f, 3.0f) == (v.Swizzle<0, 1, 2, 3>())).All());
    EXPECT_TRUE((Vector4(3.0f, 2.0f, 1.0f, 0.0f) == (v.Swizzle<3, 2, 1, 0>())).All());
    EXPECT_TRUE((Vector4(0.0f, 0.0f, 0.0f, 0.0f) == (v.Swizzle<0, 0, 0, 0>())).All());
    EXPECT_TRUE((Vector4(1.0f, 1.0f, 1.0f, 1.0f) == (v.Swizzle<1, 1, 1, 1>())).All());
    EXPECT_TRUE((Vector4(2.0f, 2.0f, 2.0f, 2.0f) == (v.Swizzle<2, 2, 2, 2>())).All());
    EXPECT_TRUE((Vector4(3.0f, 3.0f, 3.0f, 3.0f) == (v.Swizzle<3, 3, 3, 3>())).All());
    EXPECT_TRUE((Vector4(1.0f, 0.0f, 0.0f, 0.0f) == (v.Swizzle<1, 0, 0, 0>())).All());
    EXPECT_TRUE((Vector4(0.0f, 1.0f, 0.0f, 0.0f) == (v.Swizzle<0, 1, 0, 0>())).All());
    EXPECT_TRUE((Vector4(0.0f, 0.0f, 1.0f, 0.0f) == (v.Swizzle<0, 0, 1, 0>())).All());
    EXPECT_TRUE((Vector4(0.0f, 0.0f, 0.0f, 1.0f) == (v.Swizzle<0, 0, 0, 1>())).All());
    EXPECT_TRUE((Vector4(2.0f, 0.0f, 0.0f, 0.0f) == (v.Swizzle<2, 0, 0, 0>())).All());
    EXPECT_TRUE((Vector4(0.0f, 2.0f, 0.0f, 0.0f) == (v.Swizzle<0, 2, 0, 0>())).All());
    EXPECT_TRUE((Vector4(0.0f, 0.0f, 2.0f, 0.0f) == (v.Swizzle<0, 0, 2, 0>())).All());
    EXPECT_TRUE((Vector4(0.0f, 0.0f, 0.0f, 2.0f) == (v.Swizzle<0, 0, 0, 2>())).All());
    EXPECT_TRUE((Vector4(3.0f, 0.0f, 0.0f, 0.0f) == (v.Swizzle<3, 0, 0, 0>())).All());
    EXPECT_TRUE((Vector4(0.0f, 3.0f, 0.0f, 0.0f) == (v.Swizzle<0, 3, 0, 0>())).All());
    EXPECT_TRUE((Vector4(0.0f, 0.0f, 3.0f, 0.0f) == (v.Swizzle<0, 0, 3, 0>())).All());
    EXPECT_TRUE((Vector4(0.0f, 0.0f, 0.0f, 3.0f) == (v.Swizzle<0, 0, 0, 3>())).All());
}

TEST(MathTest, Vector4_ChangeSign)
{
    const Vector4 v(0.5f, 1.0f, 2.0f, 3.0f);

    EXPECT_TRUE((Vector4(0.5f, 1.0f, 2.0f, 3.0f) == (v.ChangeSign<false, false, false, false>())).All());
    EXPECT_TRUE((Vector4(0.5f, 1.0f, 2.0f, -3.0f) == (v.ChangeSign<false, false, false, true>())).All());
    EXPECT_TRUE((Vector4(0.5f, 1.0f, -2.0f, 3.0f) == (v.ChangeSign<false, false, true, false>())).All());
    EXPECT_TRUE((Vector4(0.5f, 1.0f, -2.0f, -3.0f) == (v.ChangeSign<false, false, true, true>())).All());
    EXPECT_TRUE((Vector4(0.5f, -1.0f, 2.0f, 3.0f) == (v.ChangeSign<false, true, false, false>())).All());
    EXPECT_TRUE((Vector4(0.5f, -1.0f, 2.0f, -3.0f) == (v.ChangeSign<false, true, false, true>())).All());
    EXPECT_TRUE((Vector4(0.5f, -1.0f, -2.0f, 3.0f) == (v.ChangeSign<false, true, true, false>())).All());
    EXPECT_TRUE((Vector4(0.5f, -1.0f, -2.0f, -3.0f) == (v.ChangeSign<false, true, true, true>())).All());
    EXPECT_TRUE((Vector4(-0.5f, 1.0f, 2.0f, 3.0f) == (v.ChangeSign<true, false, false, false>())).All());
    EXPECT_TRUE((Vector4(-0.5f, 1.0f, 2.0f, -3.0f) == (v.ChangeSign<true, false, false, true>())).All());
    EXPECT_TRUE((Vector4(-0.5f, 1.0f, -2.0f, 3.0f) == (v.ChangeSign<true, false, true, false>())).All());
    EXPECT_TRUE((Vector4(-0.5f, 1.0f, -2.0f, -3.0f) == (v.ChangeSign<true, false, true, true>())).All());
    EXPECT_TRUE((Vector4(-0.5f, -1.0f, 2.0f, 3.0f) == (v.ChangeSign<true, true, false, false>())).All());
    EXPECT_TRUE((Vector4(-0.5f, -1.0f, 2.0f, -3.0f) == (v.ChangeSign<true, true, false, true>())).All());
    EXPECT_TRUE((Vector4(-0.5f, -1.0f, -2.0f, 3.0f) == (v.ChangeSign<true, true, true, false>())).All());
    EXPECT_TRUE((Vector4(-0.5f, -1.0f, -2.0f, -3.0f) == (v.ChangeSign<true, true, true, true>())).All());
}

TEST(MathTest, Vector4_MakeMask)
{
    const Vector4 v(1.0f, 2.0f, 3.0f, 4.0f);

    EXPECT_TRUE((Vector4(0.0f, 0.0f, 0.0f, 4.0f) == (v & Vector4::MakeMask<0,0,0,1>())).All());
    EXPECT_TRUE((Vector4(0.0f, 0.0f, 3.0f, 0.0f) == (v & Vector4::MakeMask<0,0,1,0>())).All());
    EXPECT_TRUE((Vector4(0.0f, 0.0f, 3.0f, 4.0f) == (v & Vector4::MakeMask<0,0,1,1>())).All());
    EXPECT_TRUE((Vector4(0.0f, 2.0f, 0.0f, 0.0f) == (v & Vector4::MakeMask<0,1,0,0>())).All());
    EXPECT_TRUE((Vector4(0.0f, 2.0f, 0.0f, 4.0f) == (v & Vector4::MakeMask<0,1,0,1>())).All());
    EXPECT_TRUE((Vector4(0.0f, 2.0f, 3.0f, 0.0f) == (v & Vector4::MakeMask<0,1,1,0>())).All());
    EXPECT_TRUE((Vector4(0.0f, 2.0f, 3.0f, 4.0f) == (v & Vector4::MakeMask<0,1,1,1>())).All());
    EXPECT_TRUE((Vector4(1.0f, 0.0f, 0.0f, 0.0f) == (v & Vector4::MakeMask<1,0,0,0>())).All());
    EXPECT_TRUE((Vector4(1.0f, 0.0f, 0.0f, 4.0f) == (v & Vector4::MakeMask<1,0,0,1>())).All());
    EXPECT_TRUE((Vector4(1.0f, 0.0f, 3.0f, 0.0f) == (v & Vector4::MakeMask<1,0,1,0>())).All());
    EXPECT_TRUE((Vector4(1.0f, 0.0f, 3.0f, 4.0f) == (v & Vector4::MakeMask<1,0,1,1>())).All());
    EXPECT_TRUE((Vector4(1.0f, 2.0f, 0.0f, 0.0f) == (v & Vector4::MakeMask<1,1,0,0>())).All());
    EXPECT_TRUE((Vector4(1.0f, 2.0f, 0.0f, 4.0f) == (v & Vector4::MakeMask<1,1,0,1>())).All());
    EXPECT_TRUE((Vector4(1.0f, 2.0f, 3.0f, 0.0f) == (v & Vector4::MakeMask<1,1,1,0>())).All());
}