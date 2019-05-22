#pragma once

#include "../RayLib.h"

#include "Math.h"
#include "Matrix4.h"

namespace rt {
namespace math {

/**
 * Class representing 4D quaternion. Useful for representing rotations.
 */
struct RT_ALIGN(16) Quaternion final
{
    // XYZ - vector part, W - scalar part:
    // q = f[3] + i * q[0] + j * q[1] + k * q[2]
    Vector4 q;

    RT_FORCE_INLINE Quaternion() : q(0.0f, 0.0f, 0.0f, 1.0f) { }
    RT_FORCE_INLINE Quaternion(const Quaternion&) = default;
    RT_FORCE_INLINE explicit Quaternion(const Vector4& v) : q(v) { }
    RT_FORCE_INLINE explicit Quaternion(float i, float j, float k, float s) : q(i, j, k, s) { }

    RT_FORCE_INLINE operator const Vector4&() const { return q; }
    RT_FORCE_INLINE operator Vector4&() { return q; }

    // check if quaternion is normalized and its values are valid
    bool IsValid() const;

    // Get transformed X, Y, Z axes.
    RT_FORCE_INLINE const Vector4 GetAxisX() const;
    RT_FORCE_INLINE const Vector4 GetAxisY() const;
    RT_FORCE_INLINE const Vector4 GetAxisZ() const;

    // Create null rotation quaternion.
    RT_FORCE_INLINE static const Quaternion Identity();

    // Create quaternion form axis and angle.
    // Note: Returned quaternion is normalized.
    RAYLIB_API static const Quaternion FromAxisAndAngle(const Vector4& axis, float angle);

    // Create quaternion representing rotation around X axis.
    // Note: Returned quaternion is normalized.
    RAYLIB_API static const Quaternion RotationX(float angle);

    // Create quaternion representing rotation around Y axis.
    // Note: Returned quaternion is normalized.
    RAYLIB_API static const Quaternion RotationY(float angle);

    // Create quaternion representing rotation around Z axis.
    // Note: Returned quaternion is normalized.
    RAYLIB_API static const Quaternion RotationZ(float angle);

    // Quaternion multiplication
    RAYLIB_API const Quaternion operator * (const Quaternion& q2) const;
    RT_FORCE_INLINE Quaternion& operator *= (const Quaternion& q2);

    // Turn to unit quaternion (length = 1.0f).
    RT_FORCE_INLINE Quaternion& Normalize();
    RT_FORCE_INLINE const Quaternion Normalized() const;

    // Return conjugate of quaternion
    // Equals to Inverted() if quaternion is normalized.
    RT_FORCE_INLINE const Quaternion Conjugate() const;

    // Calculate inverse of quaternion.
    RAYLIB_API const Quaternion Inverted() const;

    // Invert this quaternion.
    RAYLIB_API Quaternion& Invert();

    // Rotate a 3D vector with a quaternion.
    RAYLIB_API const Vector4 TransformVector(const Vector4& v) const;
    RAYLIB_API const Vector3x8 TransformVector(const Vector3x8& v) const;

    // Extract rotation axis and angle from a quaternion.
    // Note: This is slow.
    RAYLIB_API void ToAxis(Vector4& outAxis, float& outAngle) const;

    // Spherical interpolation of two quaternions.
    // Returns Interpolated quaternion (equal to q0 when t=0.0f and equal to q1 when t=1.0f).
    RAYLIB_API static const Quaternion Interpolate(const Quaternion& q0, const Quaternion& q1, float t);

    // Check if two quaternions are almost equal.
    RAYLIB_API static bool AlmostEqual(const Quaternion& a, const Quaternion& b, float epsilon = RT_EPSILON);

    // Create quaternion from Euler angles.
    // pitch   - rotation in X axis (in radians)
    // yaw     - rotation in Y axis (in radians)
    // roll    - rotation in Z axis (in radians)
    RAYLIB_API static const Quaternion FromEulerAngles(const Float3& angles);

    // Create quaternion from rotation matrix.
    RAYLIB_API static const Quaternion FromMatrix(const Matrix4& matrix);

    // Convert quaternion to 4x4 matrix
    RAYLIB_API const Matrix4 ToMatrix4() const;

    // Convert quaternion to Euler angles (pitch, yaw, roll).
    // Note: This is quite costly method.
    RAYLIB_API const Float3 ToEulerAngles() const;
};


} // namespace math
} // namespace rt


#include "QuaternionImpl.h"
