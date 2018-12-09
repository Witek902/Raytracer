#include "PCH.h"
#include "Camera.h"
#include "Rendering/Context.h"


namespace rt {

using namespace math;

Camera::Camera()
    : mAspectRatio(1.0f)
    , mFieldOfView(RT_PI * 60.0f / 180.0f)
    , barrelDistortionConstFactor(0.01f)
    , barrelDistortionVariableFactor(0.0f)
    , enableBarellDistortion(false)
{ }

void Camera::SetPerspective(const math::Transform& transform, Float aspectRatio, Float FoV)
{
    RT_ASSERT(transform.IsValid());
    RT_ASSERT(IsValid(aspectRatio) && aspectRatio > 0.0f);
    RT_ASSERT(IsValid(FoV) && aspectRatio > 0.0f && FoV < RT_PI);

    mTransform = transform;
    mAspectRatio = aspectRatio;
    mFieldOfView = FoV;
    mTanHalfFoV = tanf(mFieldOfView * 0.5f);
}

void Camera::SetAngularVelocity(const math::Quaternion& quat)
{
    mAngularVelocity = quat.Normalized();
    mAngularVelocityIsZero = Quaternion::AlmostEqual(mAngularVelocity, Quaternion::Identity());

    RT_ASSERT(mAngularVelocity.IsValid());
}

Transform Camera::SampleTransform(const float time) const
{
    const Vector4 position = Vector4::MulAndAdd(mLinearVelocity, time, mTransform.GetTranslation());
    const Quaternion& rotation0 = mTransform.GetRotation();

    if (mAngularVelocityIsZero)
    {
        return Transform(position, rotation0);
    }

    const Quaternion rotation1 = rotation0 * mAngularVelocity;
    const Quaternion rotation = Quaternion::Interpolate(rotation0, rotation1, time);
    return Transform(position, rotation);
}

Ray Camera::GenerateRay(const Vector4 coords, RenderingContext& context) const
{
    const Transform transform = SampleTransform(context.time);
    Vector4 offsetedCoords = 2.0f * coords - VECTOR_ONE;

    // barrel distortion
    if (barrelDistortionVariableFactor)
    {
        Vector4 radius = Vector4::Dot2V(offsetedCoords, offsetedCoords);
        radius *= (barrelDistortionConstFactor + barrelDistortionVariableFactor * context.randomGenerator.GetFloat());
        offsetedCoords = Vector4::MulAndAdd(offsetedCoords, radius, offsetedCoords);
    }

    const Vector4 screenSpaceRayDir
    (
        offsetedCoords.x * mTanHalfFoV * mAspectRatio,
        offsetedCoords.y * mTanHalfFoV,
        1.0f,
        0.0f
    );

    // calculate ray direction (ideal, without DoF)
    Vector4 direction = transform.GetRotation().TransformVector(screenSpaceRayDir);
    Vector4 origin = transform.GetTranslation();

    // depth of field
    if (mDOF.aperture > 0.001f)
    {
        const Vector4 focusPoint = Vector4::MulAndAdd(direction, mDOF.focalPlaneDistance, origin);

        const Vector4 right = transform.GetRotation().GetAxisX();
        const Vector4 up = transform.GetRotation().GetAxisY();

        // TODO different bokeh shapes, texture, etc.
        const Vector4 randomPointOnCircle = GenerateBokeh(context) * mDOF.aperture;
        origin = Vector4::MulAndAdd(randomPointOnCircle.SplatX(), right, origin);
        origin = Vector4::MulAndAdd(randomPointOnCircle.SplatY(), up, origin);

        direction = focusPoint - origin;
    }

    return Ray(origin, direction);
}

Ray_Simd8 Camera::GenerateRay_Simd8(const Vector2x8& coords, RenderingContext& context) const
{
    const Transform transform = SampleTransform(context.time);

    Vector3x8 origin(transform.GetTranslation());
    Vector2x8 offsetedCoords = coords * 2.0f - Vector2x8::One();

    const Vector4 forwardInternal = transform.GetRotation().GetAxisZ();
    const Vector4 rightInternal = transform.GetRotation().GetAxisX();
    const Vector4 upInternal = transform.GetRotation().GetAxisY();
    const Vector4 upScaled = upInternal * mTanHalfFoV;
    const Vector4 rightScaled = rightInternal * (mTanHalfFoV * mAspectRatio);

    // barrel distortion
    if (enableBarellDistortion)
    {
        Vector8 radius = Vector2x8::Dot(offsetedCoords, offsetedCoords);
        radius *= (barrelDistortionConstFactor + barrelDistortionVariableFactor * context.randomGenerator.GetFloat());
        offsetedCoords += offsetedCoords * radius;
    }

    // calculate ray direction (ideal, without DoF)
    Vector3x8 direction = Vector3x8::MulAndAdd(Vector3x8(rightScaled), offsetedCoords.x, Vector3x8(forwardInternal));
    direction = Vector3x8::MulAndAdd(Vector3x8(upScaled), offsetedCoords.y, direction);

    // depth of field
    if (mDOF.aperture > 0.001f)
    {
        const Vector3x8 focusPoint = origin + direction * mDOF.focalPlaneDistance;

        // TODO different bokeh shapes, texture, etc.
        const Vector2x8 randomPointOnCircle = context.randomGenerator.GetCircle_Simd8() * mDOF.aperture;
        origin = Vector3x8::MulAndAdd(Vector3x8(rightInternal), randomPointOnCircle.x, origin);
        origin = Vector3x8::MulAndAdd(Vector3x8(upInternal), randomPointOnCircle.y, origin);

        direction = focusPoint - origin;
    }

    return Ray_Simd8(origin, direction);
}

//////////////////////////////////////////////////////////////////////////

Vector4 Camera::GenerateBokeh(RenderingContext& context) const
{
    switch (mDOF.bokehType)
    {
    case BokehShape::Circle:
        return context.randomGenerator.GetCircle();
    case BokehShape::Hexagon:
        return context.randomGenerator.GetHexagon();
    case BokehShape::Square:
        return context.randomGenerator.GetVector4Bipolar();
    }

    return Vector4::Zero();
}

} // namespace rt
