#include "PCH.h"
#include "../Core/Math/Quaternion.h"
#include "../Core/Math/Random.h"
#include "../Core/Math/SamplingHelpers.h"

#include "gtest/gtest.h"

using namespace rt;
using namespace math;

namespace {

// test rotation (axis = [1.23, -2.45, 4.43], angle = 74 degrees)
const Vector4 testAxis = Vector4(1.23f, -2.45f, 4.43f);
const float testAngle = DegToRad(74.0f);

const Vector4 testVector0 = Vector4(2.4f, -0.12f, 47.0f);

// calculated online
const Vector4 transformedX = Vector4(0.316016f, 0.736977f, 0.597493f);        // (1,0,0) transformed with test rotation
const Vector4 transformedY = Vector4(-0.897835f, 0.435841f, -0.06272f);       // (0,1,0) transformed with test rotation
const Vector4 transformedZ = Vector4(-0.306636f, -0.51663f, 0.799417f);       // (0,0,1) transformed with test rotation
const Vector4 transformed0 = Vector4(-13.545702f, -22.565163f, 39.014123f);   // testVector0 transformed with test rotation

const float maxError = 0.0001f;

} // namespace


TEST(MathQuaternion, Equal)
{
    const Quaternion q00 = Quaternion::FromAxisAndAngle(Vector4(1.0f, 0.0f, 0.0f), RT_PI / 2.0f);
    const Quaternion q01 = Quaternion::FromAxisAndAngle(Vector4(-1.0f, 0.0f, 0.0f), -RT_PI / 2.0f);

    const Quaternion q10 = Quaternion::FromAxisAndAngle(testAxis.Normalized3(), testAngle);
    const Quaternion q11 = Quaternion::FromAxisAndAngle(-testAxis.Normalized3(), -testAngle);

    EXPECT_TRUE(Quaternion::AlmostEqual(q00, q01, maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(q00, q00, maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(q10, q10, maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(q11, q11, maxError));

    EXPECT_FALSE(Quaternion::AlmostEqual(q00, q10, maxError));
    EXPECT_FALSE(Quaternion::AlmostEqual(q00, q11, maxError));
    EXPECT_FALSE(Quaternion::AlmostEqual(q01, q10, maxError));
    EXPECT_FALSE(Quaternion::AlmostEqual(q01, q11, maxError));
}

TEST(MathQuaternion, RotationAxisX)
{
    const Quaternion q = Quaternion::RotationX(RT_PI / 2.0f);

    const Vector4 tx = q.TransformVector(Vector4(1.0f, 0.0f, 0.0f));
    const Vector4 ty = q.TransformVector(Vector4(0.0f, 1.0f, 0.0f));
    const Vector4 tz = q.TransformVector(Vector4(0.0f, 0.0f, 1.0f));
    const Vector4 t0 = q.TransformVector(testVector0);

    EXPECT_TRUE(Vector4::AlmostEqual(tx, Vector4(1.0f, 0.0f, 0.0f)));
    EXPECT_TRUE(Vector4::AlmostEqual(ty, Vector4(0.0f, 0.0f, 1.0f)));
    EXPECT_TRUE(Vector4::AlmostEqual(tz, Vector4(0.0f, -1.0f, 0.0f)));
    EXPECT_TRUE(Vector4::AlmostEqual(t0, Vector4(2.4f, -47.0f, -0.12f), maxError));
}

TEST(MathQuaternion, RotationAxisY)
{
    const Quaternion q = Quaternion::RotationY(RT_PI / 2.0f);

    const Vector4 tx = q.TransformVector(Vector4(1.0f, 0.0f, 0.0f));
    const Vector4 ty = q.TransformVector(Vector4(0.0f, 1.0f, 0.0f));
    const Vector4 tz = q.TransformVector(Vector4(0.0f, 0.0f, 1.0f));
    const Vector4 t0 = q.TransformVector(testVector0);

    EXPECT_TRUE(Vector4::AlmostEqual(tx, Vector4(0.0f, 0.0f, -1.0f)));
    EXPECT_TRUE(Vector4::AlmostEqual(ty, Vector4(0.0f, 1.0f, 0.0f)));
    EXPECT_TRUE(Vector4::AlmostEqual(tz, Vector4(1.0f, 0.0f, 0.0f)));
    EXPECT_TRUE(Vector4::AlmostEqual(t0, Vector4(47.0f, -0.12f, -2.4f), maxError));
}

TEST(MathQuaternion, RotationAxisZ)
{
    const Quaternion q = Quaternion::RotationZ(RT_PI / 2.0f);

    const Vector4 tx = q.TransformVector(Vector4(1.0f, 0.0f, 0.0f));
    const Vector4 ty = q.TransformVector(Vector4(0.0f, 1.0f, 0.0f));
    const Vector4 tz = q.TransformVector(Vector4(0.0f, 0.0f, 1.0f));
    const Vector4 t0 = q.TransformVector(testVector0);

    EXPECT_TRUE(Vector4::AlmostEqual(tx, Vector4(0.0f, 1.0f, 0.0f)));
    EXPECT_TRUE(Vector4::AlmostEqual(ty, Vector4(-1.0f, 0.0f, 0.0f)));
    EXPECT_TRUE(Vector4::AlmostEqual(tz, Vector4(0.0f, 0.0f, 1.0f)));
    EXPECT_TRUE(Vector4::AlmostEqual(t0, Vector4(0.12f, 2.4f, 47.0f), maxError));
}

TEST(MathQuaternion, RotationAxis)
{
    const Vector4 axis = testAxis.Normalized3();
    const Quaternion q = Quaternion::FromAxisAndAngle(axis, testAngle);

    const Vector4 tx = q.TransformVector(Vector4(1.0f, 0.0f, 0.0f));
    const Vector4 ty = q.TransformVector(Vector4(0.0f, 1.0f, 0.0f));
    const Vector4 tz = q.TransformVector(Vector4(0.0f, 0.0f, 1.0f));
    const Vector4 t0 = q.TransformVector(testVector0);

    EXPECT_TRUE(Vector4::AlmostEqual(tx, transformedX, maxError));
    EXPECT_TRUE(Vector4::AlmostEqual(ty, transformedY, maxError));
    EXPECT_TRUE(Vector4::AlmostEqual(tz, transformedZ, maxError));
    EXPECT_TRUE(Vector4::AlmostEqual(t0, transformed0, maxError));
}

TEST(MathQuaternion, ToMatrix)
{
    const Vector4 axis = testAxis.Normalized3();
    const Quaternion q = Quaternion::FromAxisAndAngle(axis, testAngle);

    Matrix4 m = q.ToMatrix4();
    m.r[3] = Vector4(); // zero 4th row
    ASSERT_TRUE(Vector4::AlmostEqual(m[0], transformedX, maxError));
    ASSERT_TRUE(Vector4::AlmostEqual(m[1], transformedY, maxError));
    ASSERT_TRUE(Vector4::AlmostEqual(m[2], transformedZ, maxError));

    const Vector4 t0 = m.TransformVector(testVector0);
    EXPECT_TRUE(Vector4::AlmostEqual(t0, transformed0, maxError));
}

TEST(MathQuaternion, FromMatrix)
{
    const Vector4 axis = testAxis.Normalized3();
    const Quaternion q = Quaternion::FromAxisAndAngle(axis, testAngle);

    const Matrix4 m = q.ToMatrix4();
    const Quaternion q2 = Quaternion::FromMatrix(m);

    ASSERT_TRUE(Quaternion::AlmostEqual(q, q2, maxError));

    const Vector4 tx = q2.TransformVector(Vector4(1.0f, 0.0f, 0.0f));
    const Vector4 ty = q2.TransformVector(Vector4(0.0f, 1.0f, 0.0f));
    const Vector4 tz = q2.TransformVector(Vector4(0.0f, 0.0f, 1.0f));
    const Vector4 t0 = q2.TransformVector(testVector0);

    EXPECT_TRUE(Vector4::AlmostEqual(tx, transformedX, maxError));
    EXPECT_TRUE(Vector4::AlmostEqual(ty, transformedY, maxError));
    EXPECT_TRUE(Vector4::AlmostEqual(tz, transformedZ, maxError));
    EXPECT_TRUE(Vector4::AlmostEqual(t0, transformed0, maxError));
}

TEST(MathQuaternion, ToAxis)
{
    const Vector4 axis = testAxis.Normalized3();
    const Quaternion q = Quaternion::FromAxisAndAngle(axis, testAngle);

    float angle2;
    Vector4 axis2;
    q.ToAxis(axis2, angle2);

    if (Vector4::Dot3(axis, axis2) > 0.0f)
    {
        ASSERT_TRUE(Vector4::AlmostEqual(axis, axis2, maxError));
        ASSERT_FLOAT_EQ(testAngle, angle2);
    }
    else
    {
        ASSERT_TRUE(Vector4::AlmostEqual(axis, -axis2, maxError));
        ASSERT_FLOAT_EQ(testAngle, -angle2);
    }
}

TEST(MathQuaternion, ToAxis_Identity)
{
    float angle;
    Vector4 axis;
    Quaternion::Identity().ToAxis(axis, angle);

    ASSERT_FLOAT_EQ(0.0f, angle);
}

TEST(MathQuaternion, MultiplyBasics)
{
    // test basic quaternion identities

    const Quaternion i = Quaternion(1.0f, 0.0f, 0.0f, 0.0f);
    const Quaternion j = Quaternion(0.0f, 1.0f, 0.0f, 0.0f);
    const Quaternion k = Quaternion(0.0f, 0.0f, 1.0f, 0.0f);
    const Quaternion one = Quaternion(0.0f, 0.0f, 0.0f, 1.0f);

    const Quaternion iNeg = Quaternion(-1.0f, 0.0f, 0.0f, 0.0f);
    const Quaternion jNeg = Quaternion(0.0f, -1.0f, 0.0f, 0.0f);
    const Quaternion kNeg = Quaternion(0.0f, 0.0f, -1.0f, 0.0f);
    const Quaternion oneNeg = Quaternion(0.0f, 0.0f, 0.0f, -1.0f);

    EXPECT_TRUE(Quaternion::AlmostEqual(i * i, oneNeg));    // i^2 = -1
    EXPECT_TRUE(Quaternion::AlmostEqual(i * j, k));         // i * j = k
    EXPECT_TRUE(Quaternion::AlmostEqual(i * k, jNeg));      // i * k = -j
    EXPECT_TRUE(Quaternion::AlmostEqual(i * one, i));       // i * 1 = i

    EXPECT_TRUE(Quaternion::AlmostEqual(j * i, kNeg));      // j * i = -k
    EXPECT_TRUE(Quaternion::AlmostEqual(j * j, oneNeg));    // j^2 = -1
    EXPECT_TRUE(Quaternion::AlmostEqual(j * k, i));         // j * k = i
    EXPECT_TRUE(Quaternion::AlmostEqual(j * one, j));       // j * 1 = j

    EXPECT_TRUE(Quaternion::AlmostEqual(k * i, j));         // k * i = j
    EXPECT_TRUE(Quaternion::AlmostEqual(k * j, iNeg));      // k * j = -i
    EXPECT_TRUE(Quaternion::AlmostEqual(k * k, oneNeg));    // k^k = -1
    EXPECT_TRUE(Quaternion::AlmostEqual(k * one, k));       // k * 1 = k

    EXPECT_TRUE(Quaternion::AlmostEqual(one * i, i));       // 1 * i = i
    EXPECT_TRUE(Quaternion::AlmostEqual(one * j, j));       // 1 * j = j
    EXPECT_TRUE(Quaternion::AlmostEqual(one * k, k));       // 1 * k = k
    EXPECT_TRUE(Quaternion::AlmostEqual(one * one, one));   // 1 * 1 = 1
}

TEST(MathQuaternion, MultiplyRotations)
{
    EXPECT_TRUE(Quaternion::AlmostEqual(Quaternion::Identity() * Quaternion::Identity(), Quaternion::Identity()));

    // 90 degree rotations (CW)
    const Quaternion xRot = Quaternion::RotationX(RT_PI / 2.0f);
    const Quaternion yRot = Quaternion::RotationY(RT_PI / 2.0f);
    const Quaternion zRot = Quaternion::RotationZ(RT_PI / 2.0f);

    // 180 degree rotations
    const Quaternion xRevRot = Quaternion::RotationX(-RT_PI / 2.0f);
    const Quaternion yRevRot = Quaternion::RotationY(-RT_PI / 2.0f);
    const Quaternion zRevRot = Quaternion::RotationZ(-RT_PI / 2.0f);

    // 90 degree rotation (CCW)
    const Quaternion xRot2 = Quaternion::RotationX(RT_PI);
    const Quaternion yRot2 = Quaternion::RotationY(RT_PI);
    const Quaternion zRot2 = Quaternion::RotationZ(RT_PI);

    // rotation in the same plane
    EXPECT_TRUE(Quaternion::AlmostEqual(xRot * xRot, xRot2, maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(yRot * yRot, yRot2, maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(zRot * zRot, zRot2, maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(xRevRot * xRot, Quaternion::Identity(), maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(yRevRot * yRot, Quaternion::Identity(), maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(zRevRot * zRot, Quaternion::Identity(), maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(xRot2 * xRot2, Quaternion::Identity(), maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(yRot2 * yRot2, Quaternion::Identity(), maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(zRot2 * zRot2, Quaternion::Identity(), maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(xRot * xRot2, xRevRot, maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(yRot * yRot2, yRevRot, maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(zRot * zRot2, zRevRot, maxError));

    // simple double 90 degree rotations
    EXPECT_TRUE(Quaternion::AlmostEqual(yRot * xRot, xRot * zRevRot, maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(xRot * yRot, yRot * zRot, maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(zRot * yRot, yRot * xRevRot, maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(xRot * zRot, yRevRot * xRot, maxError));
}

TEST(MathQuaternion, MultiplyRandomInverse)
{
    const size_t iterations = 100;
    Random random;

    for (size_t i = 0; i < iterations; ++i)
    {
        const Vector4 pointOnSphere = SamplingHelpers::GetSphere(random.GetFloat2());
        const Quaternion q = Quaternion::FromAxisAndAngle(pointOnSphere, random.GetFloat() * RT_PI);
        const Quaternion qInv = q.Inverted();

        EXPECT_TRUE(Quaternion::AlmostEqual(qInv * q, Quaternion::Identity(), maxError));
        EXPECT_TRUE(Quaternion::AlmostEqual(q * qInv, Quaternion::Identity(), maxError));
    }
}

TEST(MathQuaternion, FromEulerAngles)
{
    const float angle = DegToRad(10.0f);
    ASSERT_TRUE(Quaternion::AlmostEqual(Quaternion::FromEulerAngles(Float3(angle, 0.0f, 0.0f)), Quaternion::RotationX(angle), maxError));
    ASSERT_TRUE(Quaternion::AlmostEqual(Quaternion::FromEulerAngles(Float3(0.0f, angle, 0.0f)), Quaternion::RotationY(angle), maxError));
    ASSERT_TRUE(Quaternion::AlmostEqual(Quaternion::FromEulerAngles(Float3(0.0f, 0.0f, angle)), Quaternion::RotationZ(angle), maxError));
    ASSERT_TRUE(Quaternion::AlmostEqual(Quaternion::FromEulerAngles(Float3(-angle, 0.0f, 0.0f)), Quaternion::RotationX(-angle), maxError));
    ASSERT_TRUE(Quaternion::AlmostEqual(Quaternion::FromEulerAngles(Float3(0.0f, -angle, 0.0f)), Quaternion::RotationY(-angle), maxError));
    ASSERT_TRUE(Quaternion::AlmostEqual(Quaternion::FromEulerAngles(Float3(0.0f, 0.0f, -angle)), Quaternion::RotationZ(-angle), maxError));

    const float yawAngle = DegToRad(10.0f);
    const float pitchAngle = DegToRad(20.0f);
    const float rollAngle = DegToRad(34.0f);

    const Quaternion pitchRot = Quaternion::RotationX(pitchAngle);
    const Quaternion yawRot = Quaternion::RotationY(yawAngle);
    const Quaternion rollRot = Quaternion::RotationZ(rollAngle);

    EXPECT_TRUE(Quaternion::AlmostEqual(Quaternion::FromEulerAngles(Float3(pitchAngle, yawAngle, 0.0f)), yawRot * pitchRot, 0.001f));
    EXPECT_TRUE(Quaternion::AlmostEqual(Quaternion::FromEulerAngles(Float3(pitchAngle, 0.0f, rollAngle)), pitchRot * rollRot, 0.001f));
    EXPECT_TRUE(Quaternion::AlmostEqual(Quaternion::FromEulerAngles(Float3(0.0f, yawAngle, rollAngle)), yawRot * rollRot, 0.001f));
    EXPECT_TRUE(Quaternion::AlmostEqual(Quaternion::FromEulerAngles(Float3(pitchAngle, yawAngle, rollAngle)), yawRot * pitchRot * rollRot, 0.001f));
}

//TEST(MathQuaternion, ToEulerAngles)
//{
//    const float angle = DegToRad(10.0f);
//
//    float pitch, yaw, roll;
//
//    Quaternion::RotationX(angle).ToAngles(pitch, yaw, roll);
//    EXPECT_NEAR(angle, pitch, maxError);
//    EXPECT_NEAR(0.0f, yaw, maxError);
//    EXPECT_NEAR(0.0f, roll, maxError);
//
//    Quaternion::RotationY(angle).ToAngles(pitch, yaw, roll);
//    EXPECT_NEAR(0.0f, pitch, maxError);
//    EXPECT_NEAR(angle, yaw, maxError);
//    EXPECT_NEAR(0.0f, roll, maxError);
//
//    Quaternion::RotationZ(angle).ToAngles(pitch, yaw, roll);
//    EXPECT_NEAR(0.0f, pitch, maxError);
//    EXPECT_NEAR(0.0f, yaw, maxError);
//    EXPECT_NEAR(angle, roll, maxError);
//
//
//    const float yawAngle = DegToRad(10.0f);
//    const float pitchAngle = DegToRad(20.0f);
//    const float rollAngle = DegToRad(34.0f);
//    const Quaternion q = Quaternion::FromEulerAngles(pitchAngle, yawAngle, rollAngle);
//    q.ToAngles(pitch, yaw, roll);
//
//    EXPECT_NEAR(pitchAngle, pitch, maxError);
//    EXPECT_NEAR(yawAngle, yaw, maxError);
//    EXPECT_NEAR(rollAngle, roll, maxError);
//}