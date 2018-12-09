#include "PCH.h"
#include "Quaternion.h"
#include "Transcendental.h"

namespace rt {
namespace math {

bool Quaternion::IsValid() const
{
    if (q.IsNaN() || q.IsInfinite())
        return false;
    
    // check if normalized
    return Abs(q.SqrLength4() - 1.0f) < 0.001f;
}

Quaternion Quaternion::FromAxisAndAngle(const Vector4& axis, float angle)
{
    angle *= 0.5f;
    Quaternion q = Quaternion(axis * Sin(angle));
    q.q.w = Cos(angle);
    return q;
}

Quaternion Quaternion::RotationX(float angle)
{
    angle *= 0.5f;
    return Quaternion(Sin(angle), 0.0f, 0.0f, Cos(angle));
}

Quaternion Quaternion::RotationY(float angle)
{
    angle *= 0.5f;
    return Quaternion(0.0f, Sin(angle), 0.0f, Cos(angle));
}

Quaternion Quaternion::RotationZ(float angle)
{
    angle *= 0.5f;
    return Quaternion(0.0f, 0.0f, Sin(angle), Cos(angle));
}

Quaternion Quaternion::operator * (const Quaternion& b) const
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

Quaternion Quaternion::Inverted() const
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

Vector4 Quaternion::TransformVector(const Vector4& v) const
{
    // based on identity:
    //
    // t = 2 * cross(q.xyz, v)
    // v' = v + q.w * t + cross(q.xyz, t)

    Vector4 t = Vector4::Cross3(q, v);
    t = t + t;
    return Vector4::MulAndAdd(t, q.w, v) + Vector4::Cross3(q, t);
}

Vector3x8 Quaternion::TransformVector(const Vector3x8& v) const
{
    const Vector3x8 q8(q);
    Vector3x8 t = Vector3x8::Cross(q8, v);
    t = t + t;
    return v + t * q.w + Vector3x8::Cross(q8, t);
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
    if (s >= 0.001)
    {
        outAxis /= s;
    }

    outAxis.w = 0.0f;
}
Quaternion Quaternion::Interpolate(const Quaternion& q0, const Quaternion& q1, float t)
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

Quaternion Quaternion::FromAngles(float pitch, float yaw, float roll)
{
    // based on: https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles

    pitch *= 0.5f;
    yaw *= 0.5f;
    roll *= 0.5f;

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

void Quaternion::ToAngles(float& outPitch, float& outYaw, float& outRoll) const
{
    // based on: https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles

    float t0 = 2.0f * (q[3] * q[2] + q[0] * q[1]);
    float t2 = 2.0f * (q[3] * q[0] - q[1] * q[2]);
    float t3 = 2.0f * (q[3] * q[1] + q[2] * q[0]);
    float t1 = 1.0f - 2.0f * (q[2] * q[2] + q[0] * q[0]);
    float t4 = 1.0f - 2.0f * (q[0] * q[0] + q[1] * q[1]);

    t2 = Clamp(t2, -1.0f, 1.0f);

    outRoll = atan2f(t0, t1);
    outPitch = asinf(t2);
    outYaw = atan2f(t3, t4);
}


} // namespace math
} // namespace rt
