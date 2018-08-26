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
    result.mRotation = mRotation.Inverted().Normalized();
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
