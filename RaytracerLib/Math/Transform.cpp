#include "PCH.h"
#include "Transform.h"


namespace rt {
namespace math {


Transform Transform::operator * (const Transform& other) const
{
    Transform result;
    result.mTranslation = mRotation.TransformVector(other.mTranslation) + mTranslation;
    result.mRotation = mRotation * other.mRotation;
    return result;
}

Transform Transform::Inverted() const
{
    Transform result;
    result.mRotation = mRotation.Inverted();
    result.mTranslation = result.mRotation.TransformVector(-mTranslation);
    return result;
}

Transform& Transform::Invert()
{
    *this = Inverted();
    return *this;
}

Vector4 Transform::TransformPoint(const Vector4& p) const
{
    return mRotation.TransformVector(p) + mTranslation;
}

Vector3x8 Transform::TransformPoint(const Vector3x8& p) const
{
    return mRotation.TransformVector(p) + Vector3x8(mTranslation);
}

Vector4 Transform::TransformVector(const Vector4& v) const
{
    return mRotation.TransformVector(v);
}

Vector3x8 Transform::TransformVector(const Vector3x8& v) const
{
    return mRotation.TransformVector(v);
}

Box Transform::TransformBox(const Box& box) const
{
    // based on:
    // http://dev.theomader.com/transform-bounding-boxes/

    const Vector4 xa = mRotation.GetAxisX() * box.min.x;
    const Vector4 xb = mRotation.GetAxisX() * box.max.x;
    const Vector4 ya = mRotation.GetAxisY() * box.min.y;
    const Vector4 yb = mRotation.GetAxisY() * box.max.y;
    const Vector4 za = mRotation.GetAxisZ() * box.min.z;
    const Vector4 zb = mRotation.GetAxisZ() * box.max.z;

    return Box(
        Vector4::Min(xa, xb) + Vector4::Min(ya, yb) + Vector4::Min(za, zb) + mTranslation,
        Vector4::Max(xa, xb) + Vector4::Max(ya, yb) + Vector4::Max(za, zb) + mTranslation
    );
}

Transform Transform::Interpolate(const Transform& t0, const Transform& t1, float t)
{
    return Transform(Vector4::Lerp(t0.mTranslation, t1.mTranslation, t), Quaternion::Interpolate(t0.mRotation, t1.mRotation, t));
}

bool Transform::AlmostEqual(const Transform& a, const Transform& b, float epsilon)
{
    if (!Vector4::AlmostEqual(a.mTranslation, b.mTranslation, epsilon))
        return false;

    return Quaternion::AlmostEqual(a.mRotation, b.mRotation, epsilon);
}


} // namespace math
} // namespace rt
