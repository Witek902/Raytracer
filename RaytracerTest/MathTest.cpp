#include "PCH.h"
#include "../RaytracerLib/Math/Vector4.h"

#include "gtest/gtest.h"

using namespace rt::math;


TEST(Math, MinMax_Int)
{
    EXPECT_EQ(1, Min(1, 2));
    EXPECT_EQ(1, Min(2, 1));
    EXPECT_EQ(2, Max(1, 2));
    EXPECT_EQ(2, Max(2, 1));

    EXPECT_EQ(1, Min(1, 2, 3));
    EXPECT_EQ(3, Max(1, 2, 3));
    EXPECT_EQ(1, Min(1, 3, 2));
    EXPECT_EQ(3, Max(1, 3, 2));
    EXPECT_EQ(1, Min(3, 1, 2));
    EXPECT_EQ(3, Max(3, 1, 2));
    EXPECT_EQ(1, Min(3, 2, 1));
    EXPECT_EQ(3, Max(3, 2, 1));
    EXPECT_EQ(1, Min(2, 1, 3));
    EXPECT_EQ(3, Max(2, 1, 3));
    EXPECT_EQ(1, Min(2, 3, 1));
    EXPECT_EQ(3, Max(2, 3, 1));
}

TEST(Math, MinMax_Float)
{
    EXPECT_EQ(1.0f, Min(1.0f, 2.0f));
    EXPECT_EQ(1.0f, Min(2.0f, 1.0f));
    EXPECT_EQ(2.0f, Max(1.0f, 2.0f));
    EXPECT_EQ(2.0f, Max(2.0f, 1.0f));

    EXPECT_EQ(1.0f, Min(1.0f, 2.0f, 3.0f));
    EXPECT_EQ(3.0f, Max(1.0f, 2.0f, 3.0f));
    EXPECT_EQ(1.0f, Min(1.0f, 3.0f, 2.0f));
    EXPECT_EQ(3.0f, Max(1.0f, 3.0f, 2.0f));
    EXPECT_EQ(1.0f, Min(3.0f, 1.0f, 2.0f));
    EXPECT_EQ(3.0f, Max(3.0f, 1.0f, 2.0f));
    EXPECT_EQ(1.0f, Min(3.0f, 2.0f, 1.0f));
    EXPECT_EQ(3.0f, Max(3.0f, 2.0f, 1.0f));
    EXPECT_EQ(1.0f, Min(2.0f, 1.0f, 3.0f));
    EXPECT_EQ(3.0f, Max(2.0f, 1.0f, 3.0f));
    EXPECT_EQ(1.0f, Min(2.0f, 3.0f, 1.0f));
    EXPECT_EQ(3.0f, Max(2.0f, 3.0f, 1.0f));
}

TEST(Math, MinMax_Double)
{
    EXPECT_EQ(1.0, Min(1.0, 2.0));
    EXPECT_EQ(1.0, Min(2.0, 1.0));
    EXPECT_EQ(2.0, Max(1.0, 2.0));
    EXPECT_EQ(2.0, Max(2.0, 1.0));

    EXPECT_EQ(1.0, Min(1.0, 2.0, 3.0));
    EXPECT_EQ(3.0, Max(1.0, 2.0, 3.0));
    EXPECT_EQ(1.0, Min(1.0, 3.0, 2.0));
    EXPECT_EQ(3.0, Max(1.0, 3.0, 2.0));
    EXPECT_EQ(1.0, Min(3.0, 1.0, 2.0));
    EXPECT_EQ(3.0, Max(3.0, 1.0, 2.0));
    EXPECT_EQ(1.0, Min(3.0, 2.0, 1.0));
    EXPECT_EQ(3.0, Max(3.0, 2.0, 1.0));
    EXPECT_EQ(1.0, Min(2.0, 1.0, 3.0));
    EXPECT_EQ(3.0, Max(2.0, 1.0, 3.0));
    EXPECT_EQ(1.0, Min(2.0, 3.0, 1.0));
    EXPECT_EQ(3.0, Max(2.0, 3.0, 1.0));
}

//////////////////////////////////////////////////////////////////////////

TEST(Math, Abs_Int)
{
    EXPECT_EQ(0, Abs(0));
    EXPECT_EQ(123, Abs(123));
    EXPECT_EQ(123, Abs(-123));
}

TEST(Math, Abs_Float)
{
    EXPECT_EQ(0.0f, Abs(0.0f));
    EXPECT_EQ(0.0f, Abs(-0.0f));
    EXPECT_EQ(123.0f, Abs(123.0f));
    EXPECT_EQ(123.0f, Abs(-123.0f));
}

TEST(Math, Abs_Double)
{
    EXPECT_EQ(0.0, Abs(0.0));
    EXPECT_EQ(0.0, Abs(-0.0));
    EXPECT_EQ(123.0, Abs(123.0));
    EXPECT_EQ(123.0, Abs(-123.0));
}

//////////////////////////////////////////////////////////////////////////

TEST(Math, CopySign_Float)
{
    EXPECT_EQ(0.0f, CopySign(0.0f, 0.0f));
    EXPECT_EQ(1.0f, CopySign(1.0f, 123.0f));
    EXPECT_EQ(-1.0f, CopySign(1.0f, -123.0f));
    EXPECT_EQ(1.0f, CopySign(-1.0f, 123.0f));
    EXPECT_EQ(-1.0f, CopySign(-1.0f, -123.0f));
}

TEST(Math, CopySign_Double)
{
    EXPECT_EQ(0.0, CopySign(0.0, 0.0));
    EXPECT_EQ(1.0, CopySign(1.0, 123.0));
    EXPECT_EQ(-1.0, CopySign(1.0, -123.0));
    EXPECT_EQ(1.0, CopySign(-1.0, 123.0));
    EXPECT_EQ(-1.0, CopySign(-1.0, -123.0));
}

//////////////////////////////////////////////////////////////////////////

TEST(Math, Clamp_Float)
{
    EXPECT_EQ(1.0f, Clamp(-1.0f, 1.0f, 2.0f));
    EXPECT_EQ(1.5f, Clamp(1.5f, 1.0f, 2.0f));
    EXPECT_EQ(2.0f, Clamp(123.0f, 1.0f, 2.0f));
}

TEST(Math, Clamp_Int)
{
    EXPECT_EQ(1, Clamp(-1, 1, 2));
    EXPECT_EQ(1, Clamp(1, 1, 2));
    EXPECT_EQ(2, Clamp(123, 1, 2));
}

//////////////////////////////////////////////////////////////////////////

TEST(Math, Lerp_Float)
{
    EXPECT_NEAR(1.0f, Lerp(1.0f, 2.0f, 0.0f), FLT_EPSILON);
    EXPECT_NEAR(1.7f, Lerp(1.0f, 2.0f, 0.7f), FLT_EPSILON);
    EXPECT_NEAR(2.0f, Lerp(1.0f, 2.0f, 1.0f), FLT_EPSILON);
}

//////////////////////////////////////////////////////////////////////////

TEST(Math, IsPowerOfTwo)
{
    EXPECT_TRUE(PowerOfTwo(1u));
    EXPECT_TRUE(PowerOfTwo(2u));
    EXPECT_TRUE(PowerOfTwo(4u));
    EXPECT_TRUE(PowerOfTwo(8u));
    EXPECT_TRUE(PowerOfTwo(128u));
    EXPECT_TRUE(PowerOfTwo(9223372036854775808u)); // 2^63

    EXPECT_FALSE(PowerOfTwo(0u));
    EXPECT_FALSE(PowerOfTwo(3u));
    EXPECT_FALSE(PowerOfTwo(143u));
    EXPECT_FALSE(PowerOfTwo(127u));
    EXPECT_FALSE(PowerOfTwo(129u));
    EXPECT_FALSE(PowerOfTwo(0xFFFFFFFF));
}

TEST(Math, IsNan_Float)
{
    EXPECT_TRUE(IsNaN(std::numeric_limits<float>::quiet_NaN()));
    EXPECT_TRUE(IsNaN(-std::numeric_limits<float>::quiet_NaN()));

    EXPECT_FALSE(IsNaN(std::numeric_limits<float>::max()));
    EXPECT_FALSE(IsNaN(std::numeric_limits<float>::min()));
    EXPECT_FALSE(IsNaN(-std::numeric_limits<float>::max()));
    EXPECT_FALSE(IsNaN(-std::numeric_limits<float>::min()));

    EXPECT_FALSE(IsNaN(-std::numeric_limits<float>::infinity()));
    EXPECT_FALSE(IsNaN(-std::numeric_limits<float>::infinity()));

    EXPECT_FALSE(IsNaN(0.0f));
    EXPECT_FALSE(IsNaN(-0.0f));
    EXPECT_FALSE(IsNaN(1.0f));
    EXPECT_FALSE(IsNaN(-1.0f));
    EXPECT_FALSE(IsNaN(123.0f));
    EXPECT_FALSE(IsNaN(-123.0f));
}

TEST(Math, IsInfinity_Float)
{
    EXPECT_FALSE(IsInfinity(std::numeric_limits<float>::quiet_NaN()));
    EXPECT_FALSE(IsInfinity(-std::numeric_limits<float>::quiet_NaN()));

    EXPECT_FALSE(IsInfinity(std::numeric_limits<float>::max()));
    EXPECT_FALSE(IsInfinity(std::numeric_limits<float>::min()));
    EXPECT_FALSE(IsInfinity(-std::numeric_limits<float>::max()));
    EXPECT_FALSE(IsInfinity(-std::numeric_limits<float>::min()));

    EXPECT_TRUE(IsInfinity(-std::numeric_limits<float>::infinity()));
    EXPECT_TRUE(IsInfinity(-std::numeric_limits<float>::infinity()));

    EXPECT_FALSE(IsInfinity(0.0f));
    EXPECT_FALSE(IsInfinity(-0.0f));
    EXPECT_FALSE(IsInfinity(1.0f));
    EXPECT_FALSE(IsInfinity(-1.0f));
    EXPECT_FALSE(IsInfinity(123.0f));
    EXPECT_FALSE(IsInfinity(-123.0f));
}

TEST(Math, Denormal_Float)
{
    volatile float value = 1.0e-20f;
    const float denormValue = value * value;

    EXPECT_EQ(0.0f, denormValue);
}