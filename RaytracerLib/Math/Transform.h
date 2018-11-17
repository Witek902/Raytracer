#pragma once

#include "../RayLib.h"

#include "Quaternion.h"
#include "Box.h"

namespace rt {
namespace math {

/**
 * Class representing 3D transformation (translation + rotation).
 */
class RT_ALIGN(16) Transform final
{
public:
    RT_FORCE_INLINE Transform()
        : mTranslation(Vector4::Zero())
    { }

    Transform(const Transform&) = default;
    Transform& operator = (const Transform&) = default;

    RT_FORCE_INLINE explicit Transform(const Vector4& translation, const Quaternion& rotation)
        : mTranslation(translation)
        , mRotation(rotation)
    { }

    RT_FORCE_INLINE explicit Transform(const Vector4& translation)
        : mTranslation(translation)
    { }

    RT_FORCE_INLINE explicit Transform(const Quaternion& rotation)
        : mRotation(rotation)
    { }

    RT_FORCE_INLINE const Vector4& GetTranslation() const { return mTranslation; }
    RT_FORCE_INLINE const Quaternion& GetRotation() const { return mRotation; }

    RT_FORCE_INLINE void SetTranslation(const Vector4& translation) { mTranslation = translation; }
    RT_FORCE_INLINE void SetRotation(const Quaternion& rotation) { mRotation = rotation; }

    /**
     * Transform composition.
     * The order is the same as for quaternions composition:
     * (a * b).Transform(x) == a.Transform(b.Transform(x))
     */
    RAYLIB_API Transform operator * (const Transform& other) const;
    RT_FORCE_INLINE Transform& operator *= (const Transform& other);

    /**
     * Calculate inverse of quaternion.
     */
    RAYLIB_API Transform Inverted() const;

    /**
     * Invert this quaternion.
     */
    RAYLIB_API Transform& Invert();

    /**
     * Transform a 3D point.
     */
    RAYLIB_API Vector4 TransformPoint(const Vector4& p) const;
    RAYLIB_API Vector3x8 TransformPoint(const Vector3x8& p) const;

    /**
     * Transform a 3D vector (direction).
     * @note Translation is ignored.
     */
    RAYLIB_API Vector4 TransformVector(const Vector4& v) const;
    RAYLIB_API Vector3x8 TransformVector(const Vector3x8& v) const;

    RAYLIB_API Box TransformBox(const Box& box) const;

    /**
     * Interpolate two transforms.
     * @note Translations are interpolated linearly, rotations - spherically.
     */
    RAYLIB_API static Transform Interpolate(const Transform& t0, const Transform& t1, float t);

    /**
     * Check if two transforms are almost equal.
     */
    RAYLIB_API static bool AlmostEqual(const Transform& a, const Transform& b, float epsilon = RT_EPSILON);

private:
    Vector4 mTranslation;
    Quaternion mRotation;
};


Transform& Transform::operator *= (const Transform& other)
{
    *this = *this * other;
    return *this;
}


} // namespace math
} // namespace rt
