#include "PCH.h"
#include "../Core/Math/Vector8.h"
#include "../Core/Math/VectorInt8.h"

using namespace rt::math;

TEST(MathTest, Vector8_Constructor0)
{
    const Vector8 v;
}

TEST(MathTest, Vector8_Constructor1)
{
    const Vector8 v(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);

    EXPECT_EQ(1.0f, v[0]);
    EXPECT_EQ(2.0f, v[1]);
    EXPECT_EQ(3.0f, v[2]);
    EXPECT_EQ(4.0f, v[3]);
    EXPECT_EQ(5.0f, v[4]);
    EXPECT_EQ(6.0f, v[5]);
    EXPECT_EQ(7.0f, v[6]);
    EXPECT_EQ(8.0f, v[7]);
}

TEST(MathTest, Vector8_Constructor2)
{
    const Vector8 v(7.0f);

    EXPECT_EQ(7.0f, v[0]);
    EXPECT_EQ(7.0f, v[1]);
    EXPECT_EQ(7.0f, v[2]);
    EXPECT_EQ(7.0f, v[3]);
    EXPECT_EQ(7.0f, v[4]);
    EXPECT_EQ(7.0f, v[5]);
    EXPECT_EQ(7.0f, v[6]);
    EXPECT_EQ(7.0f, v[7]);
}

TEST(MathTest, Vector8_ConstructorHiLow)
{
    const Vector4 vA(1.0f, 2.0f, 3.0f, 4.0f);
    const Vector4 vB(5.0f, 6.0f, 7.0f, 8.0f);

    const Vector8 v(vA, vB);

    EXPECT_EQ(1.0f, v[0]);
    EXPECT_EQ(2.0f, v[1]);
    EXPECT_EQ(3.0f, v[2]);
    EXPECT_EQ(4.0f, v[3]);
    EXPECT_EQ(5.0f, v[4]);
    EXPECT_EQ(6.0f, v[5]);
    EXPECT_EQ(7.0f, v[6]);
    EXPECT_EQ(8.0f, v[7]);
}

/*
TEST(MathTest, Vector8_Transpose8x8)
{
    Vector8 v0(00.0f, 01.0f, 02.0f, 03.0f, 04.0f, 05.0f, 06.0f, 07.0f);
    Vector8 v1(10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f, 17.0f);
    Vector8 v2(20.0f, 21.0f, 22.0f, 23.0f, 24.0f, 25.0f, 26.0f, 27.0f);
    Vector8 v3(30.0f, 31.0f, 32.0f, 33.0f, 34.0f, 35.0f, 36.0f, 37.0f);
    Vector8 v4(40.0f, 41.0f, 42.0f, 43.0f, 44.0f, 45.0f, 46.0f, 47.0f);
    Vector8 v5(50.0f, 51.0f, 52.0f, 53.0f, 54.0f, 55.0f, 56.0f, 57.0f);
    Vector8 v6(60.0f, 61.0f, 62.0f, 63.0f, 64.0f, 65.0f, 66.0f, 67.0f);
    Vector8 v7(70.0f, 71.0f, 72.0f, 73.0f, 74.0f, 75.0f, 76.0f, 77.0f);

    Vector8::Transpose8x8(v0, v1, v2, v3, v4, v5, v6, v7);

    EXPECT_TRUE((Vector8(00.0f, 10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f, 70.0f) == v0).All());
    EXPECT_TRUE((Vector8(01.0f, 11.0f, 21.0f, 31.0f, 41.0f, 51.0f, 61.0f, 71.0f) == v1).All());
    EXPECT_TRUE((Vector8(02.0f, 12.0f, 22.0f, 32.0f, 42.0f, 52.0f, 62.0f, 72.0f) == v2).All());
    EXPECT_TRUE((Vector8(03.0f, 13.0f, 23.0f, 33.0f, 43.0f, 53.0f, 63.0f, 73.0f) == v3).All());
    EXPECT_TRUE((Vector8(04.0f, 14.0f, 24.0f, 34.0f, 44.0f, 54.0f, 64.0f, 74.0f) == v4).All());
    EXPECT_TRUE((Vector8(05.0f, 15.0f, 25.0f, 35.0f, 45.0f, 55.0f, 65.0f, 75.0f) == v5).All());
    EXPECT_TRUE((Vector8(06.0f, 16.0f, 26.0f, 36.0f, 46.0f, 56.0f, 66.0f, 76.0f) == v6).All());
    EXPECT_TRUE((Vector8(07.0f, 17.0f, 27.0f, 37.0f, 47.0f, 57.0f, 67.0f, 77.0f) == v7).All());
}
*/

TEST(MathTest, Vector8Bool_Get)
{
    const VectorBool8 vec(true, false, false, false, true, true, false, true);

    EXPECT_EQ(true, vec.Get<0>());
    EXPECT_EQ(false, vec.Get<1>());
    EXPECT_EQ(false, vec.Get<2>());
    EXPECT_EQ(false, vec.Get<3>());
    EXPECT_EQ(true, vec.Get<4>());
    EXPECT_EQ(true, vec.Get<5>());
    EXPECT_EQ(false, vec.Get<6>());
    EXPECT_EQ(true, vec.Get<7>());
}

TEST(MathTest, Vector8_Arithmetics)
{
    EXPECT_TRUE((Vector8(11.0f, 22.0f, 33.0f, 44.0f, 55.0f, 66.0f, 77.0f, 88.0f)
        == Vector8(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f) + Vector8(10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f, 70.0f, 80.0f)).All());

    EXPECT_TRUE((Vector8(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f)
        == Vector8(11.0f, 22.0f, 33.0f, 44.0f, 55.0f, 66.0f, 77.0f, 88.0f) - Vector8(10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f, 70.0f, 80.0f)).All());

    EXPECT_TRUE((Vector8(10.0f, 40.0f, 90.0f, 160.0f, 250.0f, 360.0f, 490.0f, 640.0f)
        == Vector8(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f) * Vector8(10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f, 70.0f, 80.0f)).All());

    EXPECT_TRUE((Vector8(2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.0f, 14.0f, 16.0f)
        == Vector8(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f) * 2.0f).All());

    EXPECT_TRUE((Vector8(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f)
        == Vector8(2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.0f, 14.0f, 16.0f) / 2.0f).All());

    EXPECT_TRUE((Vector8(-1.0f, -2.0f, -3.0f, -4.0f, -5.0f, -6.0f, -7.0f, -8.0f)
        == -Vector8(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f)).All());
}

TEST(MathTest, Vector8_Gather)
{
    const float data[] = { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f };
    const VectorInt8 indices(5, 6, 0, 1, 10, 12, 3, 0);

    EXPECT_TRUE((Vector8(5.0f, 6.0f, 0.0f, 1.0f, 10.0f, 12.0f, 3.0f, 0.0f) == Gather8(data, indices)).All());
}
