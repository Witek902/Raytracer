#include "PCH.h"
#include "Quaternion.h"
#include "Transcendental.h"

namespace rt {
namespace math {

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
    return v + t * q[3] + Vector4::Cross3(q, t);
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

    const float s = sqrtf(1.0f - scalar * scalar);
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
        float sinOmega = sqrtf(1.0f - cosOmega * cosOmega);
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


} // namespace math
} // namespace rt
