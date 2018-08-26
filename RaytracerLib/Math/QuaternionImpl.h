#pragma once

namespace rt {
namespace math {

Quaternion Quaternion::Identity()
{
    return Quaternion(Vector4(0.0f, 0.0f, 0.0f, 1.0f));
}

Quaternion Quaternion::Conjugate() const
{
    Quaternion result;
    result.q = q.ChangeSign<true, true, true, false>();
    return result;
}

Quaternion Quaternion::Normalized() const
{
    return Quaternion(q.Normalized4());
}

Quaternion& Quaternion::Normalize()
{
    q.Normalize4();
    return *this;
}

Quaternion& Quaternion::operator *= (const Quaternion& q2)
{
    *this = *this * q2;
    return *this;
}

Vector4 Quaternion::GetAxisX() const
{
    return Vector4(
        1.0f - 2.0f * (q.y * q.y + q.z * q.z),
        2.0f * (q.x * q.y + q.w * q.z),
        2.0f * (q.x * q.z - q.w * q.y),
        0.0f
    );
}

Vector4 Quaternion::GetAxisY() const
{
    return Vector4(
        2.0f * (q.x * q.y - q.w * q.z),
        1.0f - 2.0f * (q.x * q.x + q.z * q.z),
        2.0f * (q.y * q.z + q.w * q.x),
        0.0f
    );
}

Vector4 Quaternion::GetAxisZ() const
{
    return Vector4(
        2.0f * (q.x * q.z + q.w * q.y),
        2.0f * (q.y * q.z - q.w * q.x),
        1.0f - 2.0f * (q.x * q.x + q.y * q.y),
        0.0f
    );
}


} // namespace math
} // namespace rt
