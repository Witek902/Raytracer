#include "PCH.h"
#include "../Core/Math/Matrix4.h"

#include "gtest/gtest.h"

using namespace rt::math;


namespace {

const Matrix4 matA = Matrix4(Vector4(-10.0f, 8.0f, -8.0f,  9.0f),
                             Vector4(-2.0f, 4.0f, 7.0f, 5.0f),
                             Vector4(2.0f, 9.0f, -1.0f, 3.0f),
                             Vector4(-3.0f, -1.0f, 8.0f, -9.0f));

const Matrix4 matB = Matrix4(Vector4(4.0f,   9.0f, -2.0f,  2.0f),
                             Vector4(-10.0f, 6.0f, -9.0f, -9.0f),
                             Vector4(2.0f,  -2.0f,  6.0f, -8.0f),
                             Vector4(-9.0f,  3.0f, -9.0f,  1.0f));

const Matrix4 matI = Matrix4(Vector4(1.0f, 0.0f, 0.0f, 0.0f),
                             Vector4(0.0f, 1.0f, 0.0f, 0.0f),
                             Vector4(0.0f, 0.0f, 1.0f, 0.0f),
                             Vector4(0.0f, 0.0f, 0.0f, 1.0f));

} // namespace

TEST(MathMatrix4, Simple)
{
    const Matrix4 matTransposeA = Matrix4(Vector4(-10.0f, -2.0f, 2.0f, -3.0f),
                                          Vector4(8.0f, 4.0f, 9.0f, -1.0f),
                                          Vector4(-8.0f, 7.0f, -1.0f, 8.0f),
                                          Vector4(9.0f, 5.0f, 3.0f, -9.0f));

    EXPECT_TRUE(matA[2][2] == -1.0f); // rows access
    EXPECT_FALSE(matA == matB);
    EXPECT_TRUE(matI == Matrix4::Identity());
    EXPECT_TRUE(matTransposeA == matA.Transposed());
}

TEST(MathMatrix4, Arithmetics)
{
    const Matrix4 matAB = Matrix4(Vector4(-217.0f, 1.0f,  -181.0f, -19.0f),
                                  Vector4(-79.0f,  7.0f,  -35.0f,  -91.0f),
                                  Vector4(-111.0f, 83.0f, -118.0f, -66.0f),
                                  Vector4(95.0f,  -76.0f,  144.0f, -70.0f));

    EXPECT_TRUE(matAB == matA * matB);
}
