#include "PCH.h"
#include "../Core/Math/Vector4.h"
#include "../Core/Math/VectorInt4.h"

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

TEST(MathTest, Vector4_ToFromFloat2)
{
    const Float2 f2 = vecB.ToFloat2();
    EXPECT_TRUE(f2.x == 1.0f && f2.y == 2.0f);
    EXPECT_TRUE((Vector4(f2) == Vector4(1.0f, 2.0f, 0.0f, 0.0f)).All());
}

TEST(MathTest, Vector4_ToFromFloat3)
{
    const Float3 f3 = vecB.ToFloat3();
    EXPECT_TRUE(f3.x == 1.0f && f3.y == 2.0f && f3.z == 3.0f);
    EXPECT_TRUE((Vector4(f3) == Vector4(1.0f, 2.0f, 3.0f, 0.0f)).All());
}

TEST(MathTest, Vector4_Splat)
{
    EXPECT_TRUE((Vector4(1.0f, 1.0f, 1.0f, 1.0f) == vecB.SplatX()).All());
    EXPECT_TRUE((Vector4(2.0f, 2.0f, 2.0f, 2.0f) == vecB.SplatY()).All());
    EXPECT_TRUE((Vector4(3.0f, 3.0f, 3.0f, 3.0f) == vecB.SplatZ()).All());
    EXPECT_TRUE((Vector4(4.0f, 4.0f, 4.0f, 4.0f) == vecB.SplatW()).All());
}

TEST(MathTest, Vector4_Load_2xUint8_Norm)
{
    {
        const Uint8 data[2] = { 0, 0 };
        EXPECT_TRUE((Vector4(0.0f, 0.0f, 0.0f, 0.0f) == Vector4::Load_2xUint8_Norm(data)).All());
    }
    {
        const Uint8 data[2] = { 0, 255 };
        EXPECT_TRUE((Vector4(0.0f, 1.0f, 0.0f, 0.0f) == Vector4::Load_2xUint8_Norm(data)).All());
    }
    {
        const Uint8 data[2] = { 255, 255 };
        EXPECT_TRUE((Vector4(1.0f, 1.0f, 0.0f, 0.0f) == Vector4::Load_2xUint8_Norm(data)).All());
    }
    {
        const Uint8 data[2] = { 255, 0 };
        EXPECT_TRUE((Vector4(1.0f, 0.0f, 0.0f, 0.0f) == Vector4::Load_2xUint8_Norm(data)).All());
    }
    {
        const Uint8 data[2] = { 35, 86 };
        EXPECT_TRUE((Vector4(35.0f / 255.0f, 86.0f / 255.0f, 0.0f, 0.0f) == Vector4::Load_2xUint8_Norm(data)).All());
    }
}

TEST(MathTest, Vector4_Load_4xUint8_Norm)
{
    {
        const Uint8 data[4] = { 0, 0, 0, 0 };
        EXPECT_TRUE((Vector4(0.0f, 0.0f, 0.0f, 0.0f) == Vector4::Load_4xUint8(data)).All());
    }
    {
        const Uint8 data[4] = { 255, 0, 0, 255 };
        EXPECT_TRUE((Vector4(255.0f, 0.0f, 0.0f, 255.0f) == Vector4::Load_4xUint8(data)).All());
    }
    {
        const Uint8 data[4] = { 255, 255, 255, 255 };
        EXPECT_TRUE((Vector4(255.0f, 255.0f, 255.0f, 255.0f) == Vector4::Load_4xUint8(data)).All());
    }
    {
        const Uint8 data[4] = { 35, 86, 241, 13 };
        EXPECT_TRUE((Vector4(35.0f, 86.0f, 241.0f, 13.0f) == Vector4::Load_4xUint8(data)).All());
    }
}

TEST(MathTest, Vector4_Load_2xUint16_Norm)
{
    {
        const Uint16 data[2] = { 0, 0 };
        EXPECT_TRUE((Vector4(0.0f, 0.0f, 0.0f, 0.0f) == Vector4::Load_2xUint16_Norm(data)).All());
    }
    {
        const Uint16 data[2] = { 0, 65535 };
        EXPECT_TRUE((Vector4(0.0f, 1.0f, 0.0f, 0.0f) == Vector4::Load_2xUint16_Norm(data)).All());
    }
    {
        const Uint16 data[2] = { 65535, 0 };
        EXPECT_TRUE((Vector4(1.0f, 0.0f, 0.0f, 0.0f) == Vector4::Load_2xUint16_Norm(data)).All());
    }
    {
        const Uint16 data[2] = { 65535, 65535 };
        EXPECT_TRUE((Vector4(1.0f, 1.0f, 0.0f, 0.0f) == Vector4::Load_2xUint16_Norm(data)).All());
    }
    {
        const Uint16 data[2] = { 31351, 8135 };
        EXPECT_TRUE((Vector4(31351.0f / 65535.0f, 8135.0f / 65535.0f, 0.0f, 0.0f) == Vector4::Load_2xUint16_Norm(data)).All());
    }
}

TEST(MathTest, Vector4_Load_4xUint16)
{
    {
        const Uint16 data[4] = { 0, 0, 0, 0 };
        EXPECT_TRUE((Vector4(0.0f, 0.0f, 0.0f, 0.0f) == Vector4::Load_4xUint16(data)).All());
    }
    {
        const Uint16 data[4] = { 65535, 0, 0, 65535 };
        EXPECT_TRUE((Vector4(65535.0f, 0.0f, 0.0f, 65535.0f) == Vector4::Load_4xUint16(data)).All());
    }
    {
        const Uint16 data[4] = { 65535, 65535, 65535, 65535 };
        EXPECT_TRUE((Vector4(65535.0f, 65535.0f, 65535.0f, 65535.0f) == Vector4::Load_4xUint16(data)).All());
    }
    {
        const Uint16 data[4] = { 31351, 8135, 12, 57964 };
        EXPECT_TRUE((Vector4(31351.0f, 8135.0f, 12.0f, 57964.0f) == Vector4::Load_4xUint16(data)).All());
    }
}

TEST(MathTest, Vector4_Select_Variable)
{
    Vector4 vA(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 vB(5.0f, 6.0f, 7.0f, 8.0f);

    EXPECT_TRUE((Vector4(1.0f, 2.0f, 3.0f, 4.0f) == Vector4::Select(vA, vB, VectorBool4(false, false, false, false))).All());
    EXPECT_TRUE((Vector4(5.0f, 2.0f, 3.0f, 4.0f) == Vector4::Select(vA, vB, VectorBool4(true, false, false, false))).All());
    EXPECT_TRUE((Vector4(1.0f, 6.0f, 3.0f, 4.0f) == Vector4::Select(vA, vB, VectorBool4(false, true, false, false))).All());
    EXPECT_TRUE((Vector4(1.0f, 2.0f, 7.0f, 4.0f) == Vector4::Select(vA, vB, VectorBool4(false, false, true, false))).All());
    EXPECT_TRUE((Vector4(1.0f, 2.0f, 3.0f, 8.0f) == Vector4::Select(vA, vB, VectorBool4(false, false, false, true))).All());
    EXPECT_TRUE((Vector4(5.0f, 6.0f, 7.0f, 8.0f) == Vector4::Select(vA, vB, VectorBool4(true, true, true, true))).All());
}

TEST(MathTest, Vector4_Select_Immediate)
{
    const Vector4 a(1.0f, 2.0f, 3.0f, 4.0f);
    const Vector4 b(5.0f, 6.0f, 7.0f, 8.0f);

    EXPECT_TRUE((Vector4(1.0f, 2.0f, 3.0f, 4.0f) == Vector4::Select<0, 0, 0, 0>(a, b)).All());
    EXPECT_TRUE((Vector4(5.0f, 6.0f, 7.0f, 8.0f) == Vector4::Select<1, 1, 1, 1>(a, b)).All());

    EXPECT_TRUE((Vector4(1.0f, 2.0f, 3.0f, 8.0f) == Vector4::Select<0, 0, 0, 1>(a, b)).All());
    EXPECT_TRUE((Vector4(1.0f, 2.0f, 7.0f, 4.0f) == Vector4::Select<0, 0, 1, 0>(a, b)).All());
    EXPECT_TRUE((Vector4(1.0f, 6.0f, 3.0f, 4.0f) == Vector4::Select<0, 1, 0, 0>(a, b)).All());
    EXPECT_TRUE((Vector4(5.0f, 2.0f, 3.0f, 4.0f) == Vector4::Select<1, 0, 0, 0>(a, b)).All());
}

//////////////////////////////////////////////////////////////////////////

TEST(MathTest, Vector4_Arithmetics)
{
    EXPECT_TRUE(Vector4::AlmostEqual(vecA + vecB, vecC));
    EXPECT_TRUE(Vector4::AlmostEqual(vecA - vecB, Vector4(0.0f, -1.0f, -2.0f, -3.0f)));
    EXPECT_TRUE(Vector4::AlmostEqual(vecB * vecB, vecD));
    EXPECT_TRUE(Vector4::AlmostEqual(vecC / vecB, Vector4(2.0f, 1.5f, 4.0f / 3.0f, 1.25f)));
    EXPECT_TRUE(Vector4::AlmostEqual(vecB * 2.0f, Vector4(2.0f, 4.0f, 6.0f, 8.0f)));
    EXPECT_TRUE(Vector4::AlmostEqual(vecB / 2.0f, Vector4(0.5f, 1.0f, 1.5f, 2.0f)));
    EXPECT_TRUE((Vector4::Abs(Vector4(-1.0f, -2.0f, 0.0f, 3.0f)) == Vector4(1.0f, 2.0f, 0.0f, 3.0f)).All());
}

TEST(MathTest, Vector4_Lerp)
{
    EXPECT_TRUE((Vector4::Lerp(vecA, vecB, 0.0f) == vecA).All());
    EXPECT_TRUE((Vector4::Lerp(vecA, vecB, 1.0f) == vecB).All());
    EXPECT_TRUE((Vector4::Lerp(vecA, vecB, 0.5f) == Vector4(1.0f, 1.5f, 2.0f, 2.5f)).All());
}

TEST(MathTest, Vector4_MinMax)
{
    EXPECT_TRUE((Vector4::Min(vecB, vecE) == Vector4(1.0f, 2.0f, 2.0f, 1.0f)).All());
    EXPECT_TRUE((Vector4::Min(vecE, vecB) == Vector4(1.0f, 2.0f, 2.0f, 1.0f)).All());
    EXPECT_TRUE((Vector4::Max(vecB, vecE) == Vector4(4.0f, 3.0f, 3.0f, 4.0f)).All());
    EXPECT_TRUE((Vector4::Max(vecB, vecE) == Vector4(4.0f, 3.0f, 3.0f, 4.0f)).All());
}

TEST(MathTest, Vector4_DotProduct)
{
    EXPECT_EQ(8.0f, Vector4::Dot2(vecB, vecC));
    EXPECT_EQ(20.0f, Vector4::Dot3(vecB, vecC));
    EXPECT_EQ(40.0f, Vector4::Dot4(vecB, vecC));

    EXPECT_TRUE((Vector4::Dot2V(vecB, vecC) == Vector4(8.0f)).All());
    EXPECT_TRUE((Vector4::Dot3V(vecB, vecC) == Vector4(20.0f)).All());
    EXPECT_TRUE((Vector4::Dot4V(vecB, vecC) == Vector4(40.0f)).All());
}

TEST(MathTest, Vector4_CrossProduct)
{
    EXPECT_TRUE((Vector4::Cross3(vecB, vecC) == Vector4(-1.0f, 2.0f, -1.0f, 0.0f)).All());
    EXPECT_TRUE((Vector4::Cross3(vecC, vecB) == Vector4(1.0f, -2.0f, 1.0f, 0.0f)).All());
}

TEST(MathTest, Vector4_Normalized)
{
    EXPECT_TRUE(Vector4::AlmostEqual(Vector4(1.0f, 2.0f, 3.0f, 4.0f).Normalized3() & Vector4::MakeMask<1,1,1,0>(),
                                    Vector4(1.0f / sqrtf(14.0f), 2.0f / sqrtf(14.0f), 3.0f / sqrtf(14.0f), 0.0f)));
    EXPECT_TRUE(Vector4::AlmostEqual(Vector4(1.0f, 2.0f, 3.0f, 4.0f).Normalized4(),
                                    Vector4(1.0f / sqrtf(30.0f), 2.0f / sqrtf(30.0f), 3.0f / sqrtf(30.0f), 4.0f / sqrtf(30.0f))));
}

TEST(MathTest, Vector4_Normalize3)
{
    Vector4 v = Vector4(1.0f, 2.0f, 3.0f, 4.0f);

    v.Normalize3();

    EXPECT_TRUE(Vector4::AlmostEqual(v & Vector4::MakeMask<1, 1, 1, 0>(),
                                    Vector4(1.0f / sqrtf(14.0f), 2.0f / sqrtf(14.0f), 3.0f / sqrtf(14.0f), 0.0f)));
}

TEST(MathTest, Vector4_Normalize4)
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

TEST(MathTest, Vector4_Less)
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

TEST(MathTest, Vector4_LessOrEqual)
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

TEST(MathTest, Vector4_Greater)
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

TEST(MathTest, Vector4_GreaterOrEqual)
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

TEST(MathTest, Vector4_Equal)
{
    EXPECT_EQ(VectorBool4(true, true, true, true), Vector4(1.0f, 2.0f, 3.0f, 4.0f) == Vector4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_EQ(VectorBool4(false, true, true, true), Vector4(10.0f, 2.0f, 3.0f, 4.0f) == Vector4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_EQ(VectorBool4(true, false, true, true), Vector4(1.0f, 20.0f, 3.0f, 4.0f) == Vector4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_EQ(VectorBool4(true, true, false, true), Vector4(1.0f, 2.0f, 30.0f, 4.0f) == Vector4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_EQ(VectorBool4(true, true, true, false), Vector4(1.0f, 2.0f, 3.0f, 40.0f) == Vector4(1.0f, 2.0f, 3.0f, 4.0f));
}

TEST(MathTest, Vector4_NotEqual)
{
    EXPECT_EQ(VectorBool4(true, true, true, true), Vector4(4.0f, 3.0f, 2.0f, 1.0f) != Vector4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_EQ(VectorBool4(false, true, true, true), Vector4(1.0f, 3.0f, 2.0f, 1.0f) != Vector4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_EQ(VectorBool4(true, false, true, true), Vector4(4.0f, 2.0f, 2.0f, 1.0f) != Vector4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_EQ(VectorBool4(true, true, false, true), Vector4(4.0f, 3.0f, 3.0f, 1.0f) != Vector4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_EQ(VectorBool4(true, true, true, false), Vector4(4.0f, 3.0f, 2.0f, 4.0f) != Vector4(1.0f, 2.0f, 3.0f, 4.0f));
}

//////////////////////////////////////////////////////////////////////////

TEST(MathTest, Vector4_Swizzle_Variable)
{
    const Vector4 v(0.0f, 1.0f, 2.0f, 3.0f);

    for (Int32 x = 0; x < 4; ++x)
    {
        for (Int32 y = 0; y < 4; ++y)
        {
            for (Int32 z = 0; z < 4; ++z)
            {
                for (Int32 w = 0; w < 4; ++w)
                {
                    const Vector4 s = v.Swizzle(x, y, z, w);
                    EXPECT_EQ(v[x], s.x);
                    EXPECT_EQ(v[y], s.y);
                    EXPECT_EQ(v[z], s.z);
                    EXPECT_EQ(v[w], s.w);
                }
            }
        }
    }
}

/*
TEST(MathTest, Vector4_Swizzle_Immediate)
{
    const Vector4 v(0.0f, 1.0f, 2.0f, 3.0f);

    {
        EXPECT_TRUE((Vector4(0.0f, 0.0f, 0.0f, 0.0f) == (v.Swizzle<0, 0, 0, 0>())).All());
        EXPECT_TRUE((Vector4(0.0f, 0.0f, 0.0f, 1.0f) == (v.Swizzle<0, 0, 0, 1>())).All());
        EXPECT_TRUE((Vector4(0.0f, 0.0f, 0.0f, 2.0f) == (v.Swizzle<0, 0, 0, 2>())).All());
        EXPECT_TRUE((Vector4(0.0f, 0.0f, 0.0f, 3.0f) == (v.Swizzle<0, 0, 0, 3>())).All());
        EXPECT_TRUE((Vector4(0.0f, 0.0f, 1.0f, 0.0f) == (v.Swizzle<0, 0, 1, 0>())).All());
        EXPECT_TRUE((Vector4(0.0f, 0.0f, 1.0f, 1.0f) == (v.Swizzle<0, 0, 1, 1>())).All());
        EXPECT_TRUE((Vector4(0.0f, 0.0f, 1.0f, 2.0f) == (v.Swizzle<0, 0, 1, 2>())).All());
        EXPECT_TRUE((Vector4(0.0f, 0.0f, 1.0f, 3.0f) == (v.Swizzle<0, 0, 1, 3>())).All());
        EXPECT_TRUE((Vector4(0.0f, 0.0f, 2.0f, 0.0f) == (v.Swizzle<0, 0, 2, 0>())).All());
        EXPECT_TRUE((Vector4(0.0f, 0.0f, 2.0f, 1.0f) == (v.Swizzle<0, 0, 2, 1>())).All());
        EXPECT_TRUE((Vector4(0.0f, 0.0f, 2.0f, 2.0f) == (v.Swizzle<0, 0, 2, 2>())).All());
        EXPECT_TRUE((Vector4(0.0f, 0.0f, 2.0f, 3.0f) == (v.Swizzle<0, 0, 2, 3>())).All());
        EXPECT_TRUE((Vector4(0.0f, 0.0f, 3.0f, 0.0f) == (v.Swizzle<0, 0, 3, 0>())).All());
        EXPECT_TRUE((Vector4(0.0f, 0.0f, 3.0f, 1.0f) == (v.Swizzle<0, 0, 3, 1>())).All());
        EXPECT_TRUE((Vector4(0.0f, 0.0f, 3.0f, 2.0f) == (v.Swizzle<0, 0, 3, 2>())).All());
        EXPECT_TRUE((Vector4(0.0f, 0.0f, 3.0f, 3.0f) == (v.Swizzle<0, 0, 3, 3>())).All());

        EXPECT_TRUE((Vector4(0.0f, 1.0f, 0.0f, 0.0f) == (v.Swizzle<0, 1, 0, 0>())).All());
        EXPECT_TRUE((Vector4(0.0f, 1.0f, 0.0f, 1.0f) == (v.Swizzle<0, 1, 0, 1>())).All());
        EXPECT_TRUE((Vector4(0.0f, 1.0f, 0.0f, 2.0f) == (v.Swizzle<0, 1, 0, 2>())).All());
        EXPECT_TRUE((Vector4(0.0f, 1.0f, 0.0f, 3.0f) == (v.Swizzle<0, 1, 0, 3>())).All());
        EXPECT_TRUE((Vector4(0.0f, 1.0f, 1.0f, 0.0f) == (v.Swizzle<0, 1, 1, 0>())).All());
        EXPECT_TRUE((Vector4(0.0f, 1.0f, 1.0f, 1.0f) == (v.Swizzle<0, 1, 1, 1>())).All());
        EXPECT_TRUE((Vector4(0.0f, 1.0f, 1.0f, 2.0f) == (v.Swizzle<0, 1, 1, 2>())).All());
        EXPECT_TRUE((Vector4(0.0f, 1.0f, 1.0f, 3.0f) == (v.Swizzle<0, 1, 1, 3>())).All());
        EXPECT_TRUE((Vector4(0.0f, 1.0f, 2.0f, 0.0f) == (v.Swizzle<0, 1, 2, 0>())).All());
        EXPECT_TRUE((Vector4(0.0f, 1.0f, 2.0f, 1.0f) == (v.Swizzle<0, 1, 2, 1>())).All());
        EXPECT_TRUE((Vector4(0.0f, 1.0f, 2.0f, 2.0f) == (v.Swizzle<0, 1, 2, 2>())).All());
        EXPECT_TRUE((Vector4(0.0f, 1.0f, 2.0f, 3.0f) == (v.Swizzle<0, 1, 2, 3>())).All());
        EXPECT_TRUE((Vector4(0.0f, 1.0f, 3.0f, 0.0f) == (v.Swizzle<0, 1, 3, 0>())).All());
        EXPECT_TRUE((Vector4(0.0f, 1.0f, 3.0f, 1.0f) == (v.Swizzle<0, 1, 3, 1>())).All());
        EXPECT_TRUE((Vector4(0.0f, 1.0f, 3.0f, 2.0f) == (v.Swizzle<0, 1, 3, 2>())).All());
        EXPECT_TRUE((Vector4(0.0f, 1.0f, 3.0f, 3.0f) == (v.Swizzle<0, 1, 3, 3>())).All());

        EXPECT_TRUE((Vector4(0.0f, 2.0f, 0.0f, 0.0f) == (v.Swizzle<0, 2, 0, 0>())).All());
        EXPECT_TRUE((Vector4(0.0f, 2.0f, 0.0f, 1.0f) == (v.Swizzle<0, 2, 0, 1>())).All());
        EXPECT_TRUE((Vector4(0.0f, 2.0f, 0.0f, 2.0f) == (v.Swizzle<0, 2, 0, 2>())).All());
        EXPECT_TRUE((Vector4(0.0f, 2.0f, 0.0f, 3.0f) == (v.Swizzle<0, 2, 0, 3>())).All());
        EXPECT_TRUE((Vector4(0.0f, 2.0f, 1.0f, 0.0f) == (v.Swizzle<0, 2, 1, 0>())).All());
        EXPECT_TRUE((Vector4(0.0f, 2.0f, 1.0f, 1.0f) == (v.Swizzle<0, 2, 1, 1>())).All());
        EXPECT_TRUE((Vector4(0.0f, 2.0f, 1.0f, 2.0f) == (v.Swizzle<0, 2, 1, 2>())).All());
        EXPECT_TRUE((Vector4(0.0f, 2.0f, 1.0f, 3.0f) == (v.Swizzle<0, 2, 1, 3>())).All());
        EXPECT_TRUE((Vector4(0.0f, 2.0f, 2.0f, 0.0f) == (v.Swizzle<0, 2, 2, 0>())).All());
        EXPECT_TRUE((Vector4(0.0f, 2.0f, 2.0f, 1.0f) == (v.Swizzle<0, 2, 2, 1>())).All());
        EXPECT_TRUE((Vector4(0.0f, 2.0f, 2.0f, 2.0f) == (v.Swizzle<0, 2, 2, 2>())).All());
        EXPECT_TRUE((Vector4(0.0f, 2.0f, 2.0f, 3.0f) == (v.Swizzle<0, 2, 2, 3>())).All());
        EXPECT_TRUE((Vector4(0.0f, 2.0f, 3.0f, 0.0f) == (v.Swizzle<0, 2, 3, 0>())).All());
        EXPECT_TRUE((Vector4(0.0f, 2.0f, 3.0f, 1.0f) == (v.Swizzle<0, 2, 3, 1>())).All());
        EXPECT_TRUE((Vector4(0.0f, 2.0f, 3.0f, 2.0f) == (v.Swizzle<0, 2, 3, 2>())).All());
        EXPECT_TRUE((Vector4(0.0f, 2.0f, 3.0f, 3.0f) == (v.Swizzle<0, 2, 3, 3>())).All());

        EXPECT_TRUE((Vector4(0.0f, 3.0f, 0.0f, 0.0f) == (v.Swizzle<0, 3, 0, 0>())).All());
        EXPECT_TRUE((Vector4(0.0f, 3.0f, 0.0f, 1.0f) == (v.Swizzle<0, 3, 0, 1>())).All());
        EXPECT_TRUE((Vector4(0.0f, 3.0f, 0.0f, 2.0f) == (v.Swizzle<0, 3, 0, 2>())).All());
        EXPECT_TRUE((Vector4(0.0f, 3.0f, 0.0f, 3.0f) == (v.Swizzle<0, 3, 0, 3>())).All());
        EXPECT_TRUE((Vector4(0.0f, 3.0f, 1.0f, 0.0f) == (v.Swizzle<0, 3, 1, 0>())).All());
        EXPECT_TRUE((Vector4(0.0f, 3.0f, 1.0f, 1.0f) == (v.Swizzle<0, 3, 1, 1>())).All());
        EXPECT_TRUE((Vector4(0.0f, 3.0f, 1.0f, 2.0f) == (v.Swizzle<0, 3, 1, 2>())).All());
        EXPECT_TRUE((Vector4(0.0f, 3.0f, 1.0f, 3.0f) == (v.Swizzle<0, 3, 1, 3>())).All());
        EXPECT_TRUE((Vector4(0.0f, 3.0f, 2.0f, 0.0f) == (v.Swizzle<0, 3, 2, 0>())).All());
        EXPECT_TRUE((Vector4(0.0f, 3.0f, 2.0f, 1.0f) == (v.Swizzle<0, 3, 2, 1>())).All());
        EXPECT_TRUE((Vector4(0.0f, 3.0f, 2.0f, 2.0f) == (v.Swizzle<0, 3, 2, 2>())).All());
        EXPECT_TRUE((Vector4(0.0f, 3.0f, 2.0f, 3.0f) == (v.Swizzle<0, 3, 2, 3>())).All());
        EXPECT_TRUE((Vector4(0.0f, 3.0f, 3.0f, 0.0f) == (v.Swizzle<0, 3, 3, 0>())).All());
        EXPECT_TRUE((Vector4(0.0f, 3.0f, 3.0f, 1.0f) == (v.Swizzle<0, 3, 3, 1>())).All());
        EXPECT_TRUE((Vector4(0.0f, 3.0f, 3.0f, 2.0f) == (v.Swizzle<0, 3, 3, 2>())).All());
        EXPECT_TRUE((Vector4(0.0f, 3.0f, 3.0f, 3.0f) == (v.Swizzle<0, 3, 3, 3>())).All());
    }

    {
        EXPECT_TRUE((Vector4(1.0f, 0.0f, 0.0f, 0.0f) == (v.Swizzle<1, 0, 0, 0>())).All());
        EXPECT_TRUE((Vector4(1.0f, 0.0f, 0.0f, 1.0f) == (v.Swizzle<1, 0, 0, 1>())).All());
        EXPECT_TRUE((Vector4(1.0f, 0.0f, 0.0f, 2.0f) == (v.Swizzle<1, 0, 0, 2>())).All());
        EXPECT_TRUE((Vector4(1.0f, 0.0f, 0.0f, 3.0f) == (v.Swizzle<1, 0, 0, 3>())).All());
        EXPECT_TRUE((Vector4(1.0f, 0.0f, 1.0f, 0.0f) == (v.Swizzle<1, 0, 1, 0>())).All());
        EXPECT_TRUE((Vector4(1.0f, 0.0f, 1.0f, 1.0f) == (v.Swizzle<1, 0, 1, 1>())).All());
        EXPECT_TRUE((Vector4(1.0f, 0.0f, 1.0f, 2.0f) == (v.Swizzle<1, 0, 1, 2>())).All());
        EXPECT_TRUE((Vector4(1.0f, 0.0f, 1.0f, 3.0f) == (v.Swizzle<1, 0, 1, 3>())).All());
        EXPECT_TRUE((Vector4(1.0f, 0.0f, 2.0f, 0.0f) == (v.Swizzle<1, 0, 2, 0>())).All());
        EXPECT_TRUE((Vector4(1.0f, 0.0f, 2.0f, 1.0f) == (v.Swizzle<1, 0, 2, 1>())).All());
        EXPECT_TRUE((Vector4(1.0f, 0.0f, 2.0f, 2.0f) == (v.Swizzle<1, 0, 2, 2>())).All());
        EXPECT_TRUE((Vector4(1.0f, 0.0f, 2.0f, 3.0f) == (v.Swizzle<1, 0, 2, 3>())).All());
        EXPECT_TRUE((Vector4(1.0f, 0.0f, 3.0f, 0.0f) == (v.Swizzle<1, 0, 3, 0>())).All());
        EXPECT_TRUE((Vector4(1.0f, 0.0f, 3.0f, 1.0f) == (v.Swizzle<1, 0, 3, 1>())).All());
        EXPECT_TRUE((Vector4(1.0f, 0.0f, 3.0f, 2.0f) == (v.Swizzle<1, 0, 3, 2>())).All());
        EXPECT_TRUE((Vector4(1.0f, 0.0f, 3.0f, 3.0f) == (v.Swizzle<1, 0, 3, 3>())).All());

        EXPECT_TRUE((Vector4(1.0f, 1.0f, 0.0f, 0.0f) == (v.Swizzle<1, 1, 0, 0>())).All());
        EXPECT_TRUE((Vector4(1.0f, 1.0f, 0.0f, 1.0f) == (v.Swizzle<1, 1, 0, 1>())).All());
        EXPECT_TRUE((Vector4(1.0f, 1.0f, 0.0f, 2.0f) == (v.Swizzle<1, 1, 0, 2>())).All());
        EXPECT_TRUE((Vector4(1.0f, 1.0f, 0.0f, 3.0f) == (v.Swizzle<1, 1, 0, 3>())).All());
        EXPECT_TRUE((Vector4(1.0f, 1.0f, 1.0f, 0.0f) == (v.Swizzle<1, 1, 1, 0>())).All());
        EXPECT_TRUE((Vector4(1.0f, 1.0f, 1.0f, 1.0f) == (v.Swizzle<1, 1, 1, 1>())).All());
        EXPECT_TRUE((Vector4(1.0f, 1.0f, 1.0f, 2.0f) == (v.Swizzle<1, 1, 1, 2>())).All());
        EXPECT_TRUE((Vector4(1.0f, 1.0f, 1.0f, 3.0f) == (v.Swizzle<1, 1, 1, 3>())).All());
        EXPECT_TRUE((Vector4(1.0f, 1.0f, 2.0f, 0.0f) == (v.Swizzle<1, 1, 2, 0>())).All());
        EXPECT_TRUE((Vector4(1.0f, 1.0f, 2.0f, 1.0f) == (v.Swizzle<1, 1, 2, 1>())).All());
        EXPECT_TRUE((Vector4(1.0f, 1.0f, 2.0f, 2.0f) == (v.Swizzle<1, 1, 2, 2>())).All());
        EXPECT_TRUE((Vector4(1.0f, 1.0f, 2.0f, 3.0f) == (v.Swizzle<1, 1, 2, 3>())).All());
        EXPECT_TRUE((Vector4(1.0f, 1.0f, 3.0f, 0.0f) == (v.Swizzle<1, 1, 3, 0>())).All());
        EXPECT_TRUE((Vector4(1.0f, 1.0f, 3.0f, 1.0f) == (v.Swizzle<1, 1, 3, 1>())).All());
        EXPECT_TRUE((Vector4(1.0f, 1.0f, 3.0f, 2.0f) == (v.Swizzle<1, 1, 3, 2>())).All());
        EXPECT_TRUE((Vector4(1.0f, 1.0f, 3.0f, 3.0f) == (v.Swizzle<1, 1, 3, 3>())).All());

        EXPECT_TRUE((Vector4(1.0f, 2.0f, 0.0f, 0.0f) == (v.Swizzle<1, 2, 0, 0>())).All());
        EXPECT_TRUE((Vector4(1.0f, 2.0f, 0.0f, 1.0f) == (v.Swizzle<1, 2, 0, 1>())).All());
        EXPECT_TRUE((Vector4(1.0f, 2.0f, 0.0f, 2.0f) == (v.Swizzle<1, 2, 0, 2>())).All());
        EXPECT_TRUE((Vector4(1.0f, 2.0f, 0.0f, 3.0f) == (v.Swizzle<1, 2, 0, 3>())).All());
        EXPECT_TRUE((Vector4(1.0f, 2.0f, 1.0f, 0.0f) == (v.Swizzle<1, 2, 1, 0>())).All());
        EXPECT_TRUE((Vector4(1.0f, 2.0f, 1.0f, 1.0f) == (v.Swizzle<1, 2, 1, 1>())).All());
        EXPECT_TRUE((Vector4(1.0f, 2.0f, 1.0f, 2.0f) == (v.Swizzle<1, 2, 1, 2>())).All());
        EXPECT_TRUE((Vector4(1.0f, 2.0f, 1.0f, 3.0f) == (v.Swizzle<1, 2, 1, 3>())).All());
        EXPECT_TRUE((Vector4(1.0f, 2.0f, 2.0f, 0.0f) == (v.Swizzle<1, 2, 2, 0>())).All());
        EXPECT_TRUE((Vector4(1.0f, 2.0f, 2.0f, 1.0f) == (v.Swizzle<1, 2, 2, 1>())).All());
        EXPECT_TRUE((Vector4(1.0f, 2.0f, 2.0f, 2.0f) == (v.Swizzle<1, 2, 2, 2>())).All());
        EXPECT_TRUE((Vector4(1.0f, 2.0f, 2.0f, 3.0f) == (v.Swizzle<1, 2, 2, 3>())).All());
        EXPECT_TRUE((Vector4(1.0f, 2.0f, 3.0f, 0.0f) == (v.Swizzle<1, 2, 3, 0>())).All());
        EXPECT_TRUE((Vector4(1.0f, 2.0f, 3.0f, 1.0f) == (v.Swizzle<1, 2, 3, 1>())).All());
        EXPECT_TRUE((Vector4(1.0f, 2.0f, 3.0f, 2.0f) == (v.Swizzle<1, 2, 3, 2>())).All());
        EXPECT_TRUE((Vector4(1.0f, 2.0f, 3.0f, 3.0f) == (v.Swizzle<1, 2, 3, 3>())).All());

        EXPECT_TRUE((Vector4(1.0f, 3.0f, 0.0f, 0.0f) == (v.Swizzle<1, 3, 0, 0>())).All());
        EXPECT_TRUE((Vector4(1.0f, 3.0f, 0.0f, 1.0f) == (v.Swizzle<1, 3, 0, 1>())).All());
        EXPECT_TRUE((Vector4(1.0f, 3.0f, 0.0f, 2.0f) == (v.Swizzle<1, 3, 0, 2>())).All());
        EXPECT_TRUE((Vector4(1.0f, 3.0f, 0.0f, 3.0f) == (v.Swizzle<1, 3, 0, 3>())).All());
        EXPECT_TRUE((Vector4(1.0f, 3.0f, 1.0f, 0.0f) == (v.Swizzle<1, 3, 1, 0>())).All());
        EXPECT_TRUE((Vector4(1.0f, 3.0f, 1.0f, 1.0f) == (v.Swizzle<1, 3, 1, 1>())).All());
        EXPECT_TRUE((Vector4(1.0f, 3.0f, 1.0f, 2.0f) == (v.Swizzle<1, 3, 1, 2>())).All());
        EXPECT_TRUE((Vector4(1.0f, 3.0f, 1.0f, 3.0f) == (v.Swizzle<1, 3, 1, 3>())).All());
        EXPECT_TRUE((Vector4(1.0f, 3.0f, 2.0f, 0.0f) == (v.Swizzle<1, 3, 2, 0>())).All());
        EXPECT_TRUE((Vector4(1.0f, 3.0f, 2.0f, 1.0f) == (v.Swizzle<1, 3, 2, 1>())).All());
        EXPECT_TRUE((Vector4(1.0f, 3.0f, 2.0f, 2.0f) == (v.Swizzle<1, 3, 2, 2>())).All());
        EXPECT_TRUE((Vector4(1.0f, 3.0f, 2.0f, 3.0f) == (v.Swizzle<1, 3, 2, 3>())).All());
        EXPECT_TRUE((Vector4(1.0f, 3.0f, 3.0f, 0.0f) == (v.Swizzle<1, 3, 3, 0>())).All());
        EXPECT_TRUE((Vector4(1.0f, 3.0f, 3.0f, 1.0f) == (v.Swizzle<1, 3, 3, 1>())).All());
        EXPECT_TRUE((Vector4(1.0f, 3.0f, 3.0f, 2.0f) == (v.Swizzle<1, 3, 3, 2>())).All());
        EXPECT_TRUE((Vector4(1.0f, 3.0f, 3.0f, 3.0f) == (v.Swizzle<1, 3, 3, 3>())).All());
    }

    {
        EXPECT_TRUE((Vector4(2.0f, 0.0f, 0.0f, 0.0f) == (v.Swizzle<2, 0, 0, 0>())).All());
        EXPECT_TRUE((Vector4(2.0f, 0.0f, 0.0f, 1.0f) == (v.Swizzle<2, 0, 0, 1>())).All());
        EXPECT_TRUE((Vector4(2.0f, 0.0f, 0.0f, 2.0f) == (v.Swizzle<2, 0, 0, 2>())).All());
        EXPECT_TRUE((Vector4(2.0f, 0.0f, 0.0f, 3.0f) == (v.Swizzle<2, 0, 0, 3>())).All());
        EXPECT_TRUE((Vector4(2.0f, 0.0f, 1.0f, 0.0f) == (v.Swizzle<2, 0, 1, 0>())).All());
        EXPECT_TRUE((Vector4(2.0f, 0.0f, 1.0f, 1.0f) == (v.Swizzle<2, 0, 1, 1>())).All());
        EXPECT_TRUE((Vector4(2.0f, 0.0f, 1.0f, 2.0f) == (v.Swizzle<2, 0, 1, 2>())).All());
        EXPECT_TRUE((Vector4(2.0f, 0.0f, 1.0f, 3.0f) == (v.Swizzle<2, 0, 1, 3>())).All());
        EXPECT_TRUE((Vector4(2.0f, 0.0f, 2.0f, 0.0f) == (v.Swizzle<2, 0, 2, 0>())).All());
        EXPECT_TRUE((Vector4(2.0f, 0.0f, 2.0f, 1.0f) == (v.Swizzle<2, 0, 2, 1>())).All());
        EXPECT_TRUE((Vector4(2.0f, 0.0f, 2.0f, 2.0f) == (v.Swizzle<2, 0, 2, 2>())).All());
        EXPECT_TRUE((Vector4(2.0f, 0.0f, 2.0f, 3.0f) == (v.Swizzle<2, 0, 2, 3>())).All());
        EXPECT_TRUE((Vector4(2.0f, 0.0f, 3.0f, 0.0f) == (v.Swizzle<2, 0, 3, 0>())).All());
        EXPECT_TRUE((Vector4(2.0f, 0.0f, 3.0f, 1.0f) == (v.Swizzle<2, 0, 3, 1>())).All());
        EXPECT_TRUE((Vector4(2.0f, 0.0f, 3.0f, 2.0f) == (v.Swizzle<2, 0, 3, 2>())).All());
        EXPECT_TRUE((Vector4(2.0f, 0.0f, 3.0f, 3.0f) == (v.Swizzle<2, 0, 3, 3>())).All());

        EXPECT_TRUE((Vector4(2.0f, 1.0f, 0.0f, 0.0f) == (v.Swizzle<2, 1, 0, 0>())).All());
        EXPECT_TRUE((Vector4(2.0f, 1.0f, 0.0f, 1.0f) == (v.Swizzle<2, 1, 0, 1>())).All());
        EXPECT_TRUE((Vector4(2.0f, 1.0f, 0.0f, 2.0f) == (v.Swizzle<2, 1, 0, 2>())).All());
        EXPECT_TRUE((Vector4(2.0f, 1.0f, 0.0f, 3.0f) == (v.Swizzle<2, 1, 0, 3>())).All());
        EXPECT_TRUE((Vector4(2.0f, 1.0f, 1.0f, 0.0f) == (v.Swizzle<2, 1, 1, 0>())).All());
        EXPECT_TRUE((Vector4(2.0f, 1.0f, 1.0f, 1.0f) == (v.Swizzle<2, 1, 1, 1>())).All());
        EXPECT_TRUE((Vector4(2.0f, 1.0f, 1.0f, 2.0f) == (v.Swizzle<2, 1, 1, 2>())).All());
        EXPECT_TRUE((Vector4(2.0f, 1.0f, 1.0f, 3.0f) == (v.Swizzle<2, 1, 1, 3>())).All());
        EXPECT_TRUE((Vector4(2.0f, 1.0f, 2.0f, 0.0f) == (v.Swizzle<2, 1, 2, 0>())).All());
        EXPECT_TRUE((Vector4(2.0f, 1.0f, 2.0f, 1.0f) == (v.Swizzle<2, 1, 2, 1>())).All());
        EXPECT_TRUE((Vector4(2.0f, 1.0f, 2.0f, 2.0f) == (v.Swizzle<2, 1, 2, 2>())).All());
        EXPECT_TRUE((Vector4(2.0f, 1.0f, 2.0f, 3.0f) == (v.Swizzle<2, 1, 2, 3>())).All());
        EXPECT_TRUE((Vector4(2.0f, 1.0f, 3.0f, 0.0f) == (v.Swizzle<2, 1, 3, 0>())).All());
        EXPECT_TRUE((Vector4(2.0f, 1.0f, 3.0f, 1.0f) == (v.Swizzle<2, 1, 3, 1>())).All());
        EXPECT_TRUE((Vector4(2.0f, 1.0f, 3.0f, 2.0f) == (v.Swizzle<2, 1, 3, 2>())).All());
        EXPECT_TRUE((Vector4(2.0f, 1.0f, 3.0f, 3.0f) == (v.Swizzle<2, 1, 3, 3>())).All());

        EXPECT_TRUE((Vector4(2.0f, 2.0f, 0.0f, 0.0f) == (v.Swizzle<2, 2, 0, 0>())).All());
        EXPECT_TRUE((Vector4(2.0f, 2.0f, 0.0f, 1.0f) == (v.Swizzle<2, 2, 0, 1>())).All());
        EXPECT_TRUE((Vector4(2.0f, 2.0f, 0.0f, 2.0f) == (v.Swizzle<2, 2, 0, 2>())).All());
        EXPECT_TRUE((Vector4(2.0f, 2.0f, 0.0f, 3.0f) == (v.Swizzle<2, 2, 0, 3>())).All());
        EXPECT_TRUE((Vector4(2.0f, 2.0f, 1.0f, 0.0f) == (v.Swizzle<2, 2, 1, 0>())).All());
        EXPECT_TRUE((Vector4(2.0f, 2.0f, 1.0f, 1.0f) == (v.Swizzle<2, 2, 1, 1>())).All());
        EXPECT_TRUE((Vector4(2.0f, 2.0f, 1.0f, 2.0f) == (v.Swizzle<2, 2, 1, 2>())).All());
        EXPECT_TRUE((Vector4(2.0f, 2.0f, 1.0f, 3.0f) == (v.Swizzle<2, 2, 1, 3>())).All());
        EXPECT_TRUE((Vector4(2.0f, 2.0f, 2.0f, 0.0f) == (v.Swizzle<2, 2, 2, 0>())).All());
        EXPECT_TRUE((Vector4(2.0f, 2.0f, 2.0f, 1.0f) == (v.Swizzle<2, 2, 2, 1>())).All());
        EXPECT_TRUE((Vector4(2.0f, 2.0f, 2.0f, 2.0f) == (v.Swizzle<2, 2, 2, 2>())).All());
        EXPECT_TRUE((Vector4(2.0f, 2.0f, 2.0f, 3.0f) == (v.Swizzle<2, 2, 2, 3>())).All());
        EXPECT_TRUE((Vector4(2.0f, 2.0f, 3.0f, 0.0f) == (v.Swizzle<2, 2, 3, 0>())).All());
        EXPECT_TRUE((Vector4(2.0f, 2.0f, 3.0f, 1.0f) == (v.Swizzle<2, 2, 3, 1>())).All());
        EXPECT_TRUE((Vector4(2.0f, 2.0f, 3.0f, 2.0f) == (v.Swizzle<2, 2, 3, 2>())).All());
        EXPECT_TRUE((Vector4(2.0f, 2.0f, 3.0f, 3.0f) == (v.Swizzle<2, 2, 3, 3>())).All());

        EXPECT_TRUE((Vector4(2.0f, 3.0f, 0.0f, 0.0f) == (v.Swizzle<2, 3, 0, 0>())).All());
        EXPECT_TRUE((Vector4(2.0f, 3.0f, 0.0f, 1.0f) == (v.Swizzle<2, 3, 0, 1>())).All());
        EXPECT_TRUE((Vector4(2.0f, 3.0f, 0.0f, 2.0f) == (v.Swizzle<2, 3, 0, 2>())).All());
        EXPECT_TRUE((Vector4(2.0f, 3.0f, 0.0f, 3.0f) == (v.Swizzle<2, 3, 0, 3>())).All());
        EXPECT_TRUE((Vector4(2.0f, 3.0f, 1.0f, 0.0f) == (v.Swizzle<2, 3, 1, 0>())).All());
        EXPECT_TRUE((Vector4(2.0f, 3.0f, 1.0f, 1.0f) == (v.Swizzle<2, 3, 1, 1>())).All());
        EXPECT_TRUE((Vector4(2.0f, 3.0f, 1.0f, 2.0f) == (v.Swizzle<2, 3, 1, 2>())).All());
        EXPECT_TRUE((Vector4(2.0f, 3.0f, 1.0f, 3.0f) == (v.Swizzle<2, 3, 1, 3>())).All());
        EXPECT_TRUE((Vector4(2.0f, 3.0f, 2.0f, 0.0f) == (v.Swizzle<2, 3, 2, 0>())).All());
        EXPECT_TRUE((Vector4(2.0f, 3.0f, 2.0f, 1.0f) == (v.Swizzle<2, 3, 2, 1>())).All());
        EXPECT_TRUE((Vector4(2.0f, 3.0f, 2.0f, 2.0f) == (v.Swizzle<2, 3, 2, 2>())).All());
        EXPECT_TRUE((Vector4(2.0f, 3.0f, 2.0f, 3.0f) == (v.Swizzle<2, 3, 2, 3>())).All());
        EXPECT_TRUE((Vector4(2.0f, 3.0f, 3.0f, 0.0f) == (v.Swizzle<2, 3, 3, 0>())).All());
        EXPECT_TRUE((Vector4(2.0f, 3.0f, 3.0f, 1.0f) == (v.Swizzle<2, 3, 3, 1>())).All());
        EXPECT_TRUE((Vector4(2.0f, 3.0f, 3.0f, 2.0f) == (v.Swizzle<2, 3, 3, 2>())).All());
        EXPECT_TRUE((Vector4(2.0f, 3.0f, 3.0f, 3.0f) == (v.Swizzle<2, 3, 3, 3>())).All());
    }

    {
        EXPECT_TRUE((Vector4(3.0f, 0.0f, 0.0f, 0.0f) == (v.Swizzle<3, 0, 0, 0>())).All());
        EXPECT_TRUE((Vector4(3.0f, 0.0f, 0.0f, 1.0f) == (v.Swizzle<3, 0, 0, 1>())).All());
        EXPECT_TRUE((Vector4(3.0f, 0.0f, 0.0f, 2.0f) == (v.Swizzle<3, 0, 0, 2>())).All());
        EXPECT_TRUE((Vector4(3.0f, 0.0f, 0.0f, 3.0f) == (v.Swizzle<3, 0, 0, 3>())).All());
        EXPECT_TRUE((Vector4(3.0f, 0.0f, 1.0f, 0.0f) == (v.Swizzle<3, 0, 1, 0>())).All());
        EXPECT_TRUE((Vector4(3.0f, 0.0f, 1.0f, 1.0f) == (v.Swizzle<3, 0, 1, 1>())).All());
        EXPECT_TRUE((Vector4(3.0f, 0.0f, 1.0f, 2.0f) == (v.Swizzle<3, 0, 1, 2>())).All());
        EXPECT_TRUE((Vector4(3.0f, 0.0f, 1.0f, 3.0f) == (v.Swizzle<3, 0, 1, 3>())).All());
        EXPECT_TRUE((Vector4(3.0f, 0.0f, 2.0f, 0.0f) == (v.Swizzle<3, 0, 2, 0>())).All());
        EXPECT_TRUE((Vector4(3.0f, 0.0f, 2.0f, 1.0f) == (v.Swizzle<3, 0, 2, 1>())).All());
        EXPECT_TRUE((Vector4(3.0f, 0.0f, 2.0f, 2.0f) == (v.Swizzle<3, 0, 2, 2>())).All());
        EXPECT_TRUE((Vector4(3.0f, 0.0f, 2.0f, 3.0f) == (v.Swizzle<3, 0, 2, 3>())).All());
        EXPECT_TRUE((Vector4(3.0f, 0.0f, 3.0f, 0.0f) == (v.Swizzle<3, 0, 3, 0>())).All());
        EXPECT_TRUE((Vector4(3.0f, 0.0f, 3.0f, 1.0f) == (v.Swizzle<3, 0, 3, 1>())).All());
        EXPECT_TRUE((Vector4(3.0f, 0.0f, 3.0f, 2.0f) == (v.Swizzle<3, 0, 3, 2>())).All());
        EXPECT_TRUE((Vector4(3.0f, 0.0f, 3.0f, 3.0f) == (v.Swizzle<3, 0, 3, 3>())).All());

        EXPECT_TRUE((Vector4(3.0f, 1.0f, 0.0f, 0.0f) == (v.Swizzle<3, 1, 0, 0>())).All());
        EXPECT_TRUE((Vector4(3.0f, 1.0f, 0.0f, 1.0f) == (v.Swizzle<3, 1, 0, 1>())).All());
        EXPECT_TRUE((Vector4(3.0f, 1.0f, 0.0f, 2.0f) == (v.Swizzle<3, 1, 0, 2>())).All());
        EXPECT_TRUE((Vector4(3.0f, 1.0f, 0.0f, 3.0f) == (v.Swizzle<3, 1, 0, 3>())).All());
        EXPECT_TRUE((Vector4(3.0f, 1.0f, 1.0f, 0.0f) == (v.Swizzle<3, 1, 1, 0>())).All());
        EXPECT_TRUE((Vector4(3.0f, 1.0f, 1.0f, 1.0f) == (v.Swizzle<3, 1, 1, 1>())).All());
        EXPECT_TRUE((Vector4(3.0f, 1.0f, 1.0f, 2.0f) == (v.Swizzle<3, 1, 1, 2>())).All());
        EXPECT_TRUE((Vector4(3.0f, 1.0f, 1.0f, 3.0f) == (v.Swizzle<3, 1, 1, 3>())).All());
        EXPECT_TRUE((Vector4(3.0f, 1.0f, 2.0f, 0.0f) == (v.Swizzle<3, 1, 2, 0>())).All());
        EXPECT_TRUE((Vector4(3.0f, 1.0f, 2.0f, 1.0f) == (v.Swizzle<3, 1, 2, 1>())).All());
        EXPECT_TRUE((Vector4(3.0f, 1.0f, 2.0f, 2.0f) == (v.Swizzle<3, 1, 2, 2>())).All());
        EXPECT_TRUE((Vector4(3.0f, 1.0f, 2.0f, 3.0f) == (v.Swizzle<3, 1, 2, 3>())).All());
        EXPECT_TRUE((Vector4(3.0f, 1.0f, 3.0f, 0.0f) == (v.Swizzle<3, 1, 3, 0>())).All());
        EXPECT_TRUE((Vector4(3.0f, 1.0f, 3.0f, 1.0f) == (v.Swizzle<3, 1, 3, 1>())).All());
        EXPECT_TRUE((Vector4(3.0f, 1.0f, 3.0f, 2.0f) == (v.Swizzle<3, 1, 3, 2>())).All());
        EXPECT_TRUE((Vector4(3.0f, 1.0f, 3.0f, 3.0f) == (v.Swizzle<3, 1, 3, 3>())).All());

        EXPECT_TRUE((Vector4(3.0f, 2.0f, 0.0f, 0.0f) == (v.Swizzle<3, 2, 0, 0>())).All());
        EXPECT_TRUE((Vector4(3.0f, 2.0f, 0.0f, 1.0f) == (v.Swizzle<3, 2, 0, 1>())).All());
        EXPECT_TRUE((Vector4(3.0f, 2.0f, 0.0f, 2.0f) == (v.Swizzle<3, 2, 0, 2>())).All());
        EXPECT_TRUE((Vector4(3.0f, 2.0f, 0.0f, 3.0f) == (v.Swizzle<3, 2, 0, 3>())).All());
        EXPECT_TRUE((Vector4(3.0f, 2.0f, 1.0f, 0.0f) == (v.Swizzle<3, 2, 1, 0>())).All());
        EXPECT_TRUE((Vector4(3.0f, 2.0f, 1.0f, 1.0f) == (v.Swizzle<3, 2, 1, 1>())).All());
        EXPECT_TRUE((Vector4(3.0f, 2.0f, 1.0f, 2.0f) == (v.Swizzle<3, 2, 1, 2>())).All());
        EXPECT_TRUE((Vector4(3.0f, 2.0f, 1.0f, 3.0f) == (v.Swizzle<3, 2, 1, 3>())).All());
        EXPECT_TRUE((Vector4(3.0f, 2.0f, 2.0f, 0.0f) == (v.Swizzle<3, 2, 2, 0>())).All());
        EXPECT_TRUE((Vector4(3.0f, 2.0f, 2.0f, 1.0f) == (v.Swizzle<3, 2, 2, 1>())).All());
        EXPECT_TRUE((Vector4(3.0f, 2.0f, 2.0f, 2.0f) == (v.Swizzle<3, 2, 2, 2>())).All());
        EXPECT_TRUE((Vector4(3.0f, 2.0f, 2.0f, 3.0f) == (v.Swizzle<3, 2, 2, 3>())).All());
        EXPECT_TRUE((Vector4(3.0f, 2.0f, 3.0f, 0.0f) == (v.Swizzle<3, 2, 3, 0>())).All());
        EXPECT_TRUE((Vector4(3.0f, 2.0f, 3.0f, 1.0f) == (v.Swizzle<3, 2, 3, 1>())).All());
        EXPECT_TRUE((Vector4(3.0f, 2.0f, 3.0f, 2.0f) == (v.Swizzle<3, 2, 3, 2>())).All());
        EXPECT_TRUE((Vector4(3.0f, 2.0f, 3.0f, 3.0f) == (v.Swizzle<3, 2, 3, 3>())).All());

        EXPECT_TRUE((Vector4(3.0f, 3.0f, 0.0f, 0.0f) == (v.Swizzle<3, 3, 0, 0>())).All());
        EXPECT_TRUE((Vector4(3.0f, 3.0f, 0.0f, 1.0f) == (v.Swizzle<3, 3, 0, 1>())).All());
        EXPECT_TRUE((Vector4(3.0f, 3.0f, 0.0f, 2.0f) == (v.Swizzle<3, 3, 0, 2>())).All());
        EXPECT_TRUE((Vector4(3.0f, 3.0f, 0.0f, 3.0f) == (v.Swizzle<3, 3, 0, 3>())).All());
        EXPECT_TRUE((Vector4(3.0f, 3.0f, 1.0f, 0.0f) == (v.Swizzle<3, 3, 1, 0>())).All());
        EXPECT_TRUE((Vector4(3.0f, 3.0f, 1.0f, 1.0f) == (v.Swizzle<3, 3, 1, 1>())).All());
        EXPECT_TRUE((Vector4(3.0f, 3.0f, 1.0f, 2.0f) == (v.Swizzle<3, 3, 1, 2>())).All());
        EXPECT_TRUE((Vector4(3.0f, 3.0f, 1.0f, 3.0f) == (v.Swizzle<3, 3, 1, 3>())).All());
        EXPECT_TRUE((Vector4(3.0f, 3.0f, 2.0f, 0.0f) == (v.Swizzle<3, 3, 2, 0>())).All());
        EXPECT_TRUE((Vector4(3.0f, 3.0f, 2.0f, 1.0f) == (v.Swizzle<3, 3, 2, 1>())).All());
        EXPECT_TRUE((Vector4(3.0f, 3.0f, 2.0f, 2.0f) == (v.Swizzle<3, 3, 2, 2>())).All());
        EXPECT_TRUE((Vector4(3.0f, 3.0f, 2.0f, 3.0f) == (v.Swizzle<3, 3, 2, 3>())).All());
        EXPECT_TRUE((Vector4(3.0f, 3.0f, 3.0f, 0.0f) == (v.Swizzle<3, 3, 3, 0>())).All());
        EXPECT_TRUE((Vector4(3.0f, 3.0f, 3.0f, 1.0f) == (v.Swizzle<3, 3, 3, 1>())).All());
        EXPECT_TRUE((Vector4(3.0f, 3.0f, 3.0f, 2.0f) == (v.Swizzle<3, 3, 3, 2>())).All());
        EXPECT_TRUE((Vector4(3.0f, 3.0f, 3.0f, 3.0f) == (v.Swizzle<3, 3, 3, 3>())).All());
    }
}
*/

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

TEST(MathTest, Vector4_Transpose3)
{
    Vector4 a(1.0f, 2.0f, 3.0f, 0.0f);
    Vector4 b(4.0f, 5.0f, 6.0f, 0.0f);
    Vector4 c(7.0f, 8.0f, 9.0f, 0.0f);

    Vector4::Transpose3(a, b, c);

    EXPECT_TRUE((Vector4(1.0f, 4.0f, 7.0f, 0.0f) == (a & Vector4::MakeMask<1,1,1,0>())).All());
    EXPECT_TRUE((Vector4(2.0f, 5.0f, 8.0f, 0.0f) == (b & Vector4::MakeMask<1,1,1,0>())).All());
    EXPECT_TRUE((Vector4(3.0f, 6.0f, 9.0f, 0.0f) == (c & Vector4::MakeMask<1,1,1,0>())).All());
}