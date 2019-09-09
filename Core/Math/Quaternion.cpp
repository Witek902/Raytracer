#include "PCH.h"
#include "Quaternion.h"
#include "Transcendental.h"

namespace rt {
namespace math {

bool Quaternion::IsValid() const
{
    if (!q.IsValid())
    {
        return false;
    }

    // check if normalized
    return Abs(q.SqrLength4() - 1.0f) < 0.001f;
}

const Quaternion Quaternion::FromAxisAndAngle(const Vector4& axis, float angle)
{
    angle *= 0.5f;
    Quaternion q = Quaternion(axis * Sin(angle));
    q.q.w = Cos(angle);
    return q;
}

const Quaternion Quaternion::RotationX(float angle)
{
    angle *= 0.5f;
    return Quaternion(Sin(angle), 0.0f, 0.0f, Cos(angle));
}

const Quaternion Quaternion::RotationY(float angle)
{
    angle *= 0.5f;
    return Quaternion(0.0f, Sin(angle), 0.0f, Cos(angle));
}

const Quaternion Quaternion::RotationZ(float angle)
{
    angle *= 0.5f;
    return Quaternion(0.0f, 0.0f, Sin(angle), Cos(angle));
}

const Quaternion Quaternion::operator * (const Quaternion& b) const
{
    const Vector4 a0120 = q.Swizzle<0, 1, 2, 0>();
    const Vector4 b3330 = b.q.Swizzle<3, 3, 3, 0>();
    const Vector4 t1 = a0120 * b3330;

    const Vector4 a1201 = q.Swizzle<1, 2, 0, 1>();
    const Vector4 b2011 = b.q.Swizzle<2, 0, 1, 1>();
    const Vector4 t12 = Vector4::MulAndAdd(a1201, b2011, t1);

    const Vector4 a3333 = q.Swizzle<3, 3, 3, 3>();
    const Vector4 b0123 = b.q;
    const Vector4 t0 = a3333 * b0123;

    const Vector4 a2012 = q.Swizzle<2, 0, 1, 2>();
    const Vector4 b1202 = b.q.Swizzle<1, 2, 0, 2>();
    const Vector4 t03 = Vector4::NegMulAndAdd(a2012, b1202, t0);

    return Quaternion(t03 + t12.ChangeSign<false, false, false, true>());
}

const Quaternion Quaternion::Inverted() const
{
    Quaternion result = Conjugate();
    result.q.Normalize4();
    return result;
}

Quaternion& Quaternion::Invert()
{
    *this = Conjugate();
    q.Normalize4();

    return *this;
}

const Vector4 Quaternion::TransformVector(const Vector4& v) const
{
    // based on identity:
    //
    // t = 2 * cross(q.xyz, v)
    // v' = v + q.w * t + cross(q.xyz, t)

    Vector4 t = Vector4::Cross3(q, v);
    t = t + t;
    return Vector4::MulAndAdd(t, q.w, v) + Vector4::Cross3(q, t);
}

const Vector3x8 Quaternion::TransformVector(const Vector3x8& v) const
{
    const Vector3x8 q8(q);
    Vector3x8 t = Vector3x8::Cross(q8, v);
    t = t + t;
    return Vector3x8::MulAndAdd(t, Vector8(q.w), v) + Vector3x8::Cross(q8, t);
}

void Quaternion::ToAxis(Vector4& outAxis, float& outAngle) const
{
    Quaternion normalized = *this;
    if (normalized.q[3] > 1.0f)
    {
        normalized = Normalized();
    }

    const float scalar = normalized.q[3];
    outAngle = 2.0f * acosf(scalar);

    const float s = Sqrt(1.0f - scalar * scalar);
    outAxis = normalized.q;
    if (s >= 0.001f)
    {
        outAxis /= s;
    }

    outAxis.w = 0.0f;
}

const Quaternion Quaternion::Interpolate(const Quaternion& q0, const Quaternion& q1, float t)
{
    float cosOmega = Vector4::Dot4(q0, q1);
    Vector4 new_q1 = q1;
    if (cosOmega < 0.0f)
    {
        new_q1 = -new_q1;
        cosOmega = -cosOmega;
    }

    float k0, k1;
    if (cosOmega > 0.9999f)
    {
        k0 = 1.0f - t;
        k1 = t;
    }
    else
    {
        float sinOmega = Sqrt(1.0f - cosOmega * cosOmega);
        float omega = atan2f(sinOmega, cosOmega);
        float oneOverSinOmega = 1.0f / sinOmega;
        k0 = Sin((1.0f - t) * omega) * oneOverSinOmega;
        k1 = Sin(t * omega) * oneOverSinOmega;
    }

    return Quaternion(q0.q * k0 + new_q1 * k1);
}

bool Quaternion::AlmostEqual(const Quaternion& a, const Quaternion& b, float epsilon)
{
    float d = Vector4::Dot4(a.q, b.q);
    return Abs(d) > 1.0f - epsilon;
}

const Quaternion Quaternion::FromEulerAngles(const Float3& angles)
{
    // based on: https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles

    const float pitch = angles.x * 0.5f;
    const float yaw = angles.y * 0.5f;
    const float roll = angles.z * 0.5f;

    Quaternion q;
    float t0 = Cos(yaw);
    float t1 = Sin(yaw);
    float t2 = Cos(roll);
    float t3 = Sin(roll);
    float t4 = Cos(pitch);
    float t5 = Sin(pitch);

    const Vector4 term0 = Vector4(t0, t1, t0, t0);
    const Vector4 term1 = Vector4(t2, t2, t3, t2);
    const Vector4 term2 = Vector4(t5, t4, t4, t4);

    const Vector4 term3 = Vector4(t1, -t0, -t1, t1);
    const Vector4 term4 = Vector4(t3, t3, t2, t3);
    const Vector4 term5 = Vector4(t4, t5, t5, t5);

    return Quaternion(term0 * term1 * term2 + term3 * term4 * term5);
}

const Quaternion Quaternion::FromMatrix(const Matrix4& m)
{
    const Vector4 x = Vector4(m.m[0][0]).ChangeSign<false, true, true, false>();
    const Vector4 y = Vector4(m.m[1][1]).ChangeSign<true, false, true, false>();
    const Vector4 z = Vector4(m.m[2][2]).ChangeSign<true, true, false, false>();

    Quaternion q;
    q.q = (Vector4(1.0f) + x) + (y + z);
    q.q = Vector4::Max(q.q, Vector4::Zero());
    q.q = Vector4::Sqrt(q.q) * 0.5f;

    q.q.x = CopySign(q.q.x, m.m[1][2] - m.m[2][1]);
    q.q.y = CopySign(q.q.y, m.m[2][0] - m.m[0][2]);
    q.q.z = CopySign(q.q.z, m.m[0][1] - m.m[1][0]);
    return q;
}

const Matrix4 Quaternion::ToMatrix4() const
{
    Matrix4 m;
    m.rows[0] = GetAxisX();
    m.rows[1] = GetAxisY();
    m.rows[2] = GetAxisZ();
    m.rows[3] = VECTOR_W;
    return m;
}

const Float3 Quaternion::ToEulerAngles() const
{
    // based on: https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles

    float t0 = 2.0f * (q[3] * q[2] + q[0] * q[1]);
    float t2 = 2.0f * (q[3] * q[0] - q[1] * q[2]);
    float t3 = 2.0f * (q[3] * q[1] + q[2] * q[0]);
    float t1 = 1.0f - 2.0f * (q[2] * q[2] + q[0] * q[0]);
    float t4 = 1.0f - 2.0f * (q[0] * q[0] + q[1] * q[1]);

    t2 = Clamp(t2, -1.0f, 1.0f);

    float pitch = asinf(t2);
    float yaw = atan2f(t3, t4);
    float roll = atan2f(t0, t1);

    if (pitch > RT_PI)
    {
        pitch -= 2.0f * RT_PI;
    }
    if (pitch < -RT_PI)
    {
        pitch += 2.0f * RT_PI;
    }

    if (yaw > RT_PI)
    {
        yaw -= 2.0f * RT_PI;
    }
    if (yaw < -RT_PI)
    {
        yaw += 2.0f * RT_PI;
    }

    if (roll > RT_PI)
    {
        roll -= 2.0f * RT_PI;
    }
    if (roll < -RT_PI)
    {
        roll += 2.0f * RT_PI;
    }

    return Float3{ pitch, yaw, roll };
}

} // namespace math
} // namespace rt
