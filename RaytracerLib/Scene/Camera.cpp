#include "PCH.h"
#include "Camera.h"
#include "Rendering/Context.h"


namespace rt {

using namespace math;

Camera::Camera()
    : mAspectRatio(1.0f)
    , mFieldOfView(RT_PI * 10.0f / 180.0f)
    , barrelDistortionFactor(0.0f)
{ }

void Camera::SetPerspective(const Vector4& pos, const Vector4& dir, const Vector4& up, Float aspectRatio, Float FoV)
{
    mPosition = pos;
    mForward = dir;
    mUp = up;
    mAspectRatio = aspectRatio;
    mFieldOfView = FoV;
}

void Camera::Update()
{
    mForwardInternal = mForward.Normalized3();
    mRightInternal = Vector4::Cross3(mUp, mForwardInternal).Normalized3();
    mUpInternal = Vector4::Cross3(mForwardInternal, mRightInternal).Normalized3();

    // field of view & aspect ratio
    const Float tanHalfFoV = tanf(mFieldOfView * 0.5f);
    mUpScaled = mUpInternal * tanHalfFoV;
    mRightScaled = mRightInternal * (tanHalfFoV * mAspectRatio);
}

Ray Camera::GenerateRay(const Vector4 coords, RenderingContext& context) const
{
    Vector4 origin = mPosition + mPositionDelta * context.time;
    Vector4 offsetedCoords = 2.0f * coords - VECTOR_ONE;

    // barrel distortion
    if (barrelDistortionFactor != 0.0f)
    {
        const Vector4 radius = Vector4::Dot2V(offsetedCoords, offsetedCoords);
        offsetedCoords += offsetedCoords * radius * barrelDistortionFactor;
    }

    // calculate ray direction (ideal, without DoF)
    Vector4 direction = Vector4::MulAndAdd(offsetedCoords.SplatX(), mRightScaled, mForwardInternal);
    direction = Vector4::MulAndAdd(offsetedCoords.SplatY(), mUpScaled, direction);

    // depth of field
    if (mDOF.aperture > 0.001f)
    {
        const Vector4 focusPoint = origin + mDOF.focalPlaneDistance * direction;

        // TODO different bokeh shapes, texture, etc.
        const Vector4 randomPointOnCircle = GenerateBokeh(context) * mDOF.aperture;
        origin = Vector4::MulAndAdd(randomPointOnCircle.SplatX(), mRightInternal, origin);
        origin = Vector4::MulAndAdd(randomPointOnCircle.SplatY(), mUpInternal, origin);

        direction = focusPoint - origin;
    }

    return Ray(origin, direction);
}

Ray_Simd8 Camera::GenerateRay_Simd8(const Vector2x8& coords, RenderingContext& context) const
{
    Vector3x8 origin(mPosition + mPositionDelta * context.time);
    Vector2x8 offsetedCoords = coords * 2.0f - Vector2x8::One();

    // barrel distortion
    if (barrelDistortionFactor != 0.0f)
    {
        const Vector8 radius = Vector2x8::Dot(offsetedCoords, offsetedCoords);
        offsetedCoords += offsetedCoords * (radius * barrelDistortionFactor);
    }

    // calculate ray direction (ideal, without DoF)
    Vector3x8 direction = Vector3x8::MulAndAdd(Vector3x8(mRightScaled), offsetedCoords.x, Vector3x8(mForwardInternal));
    direction = Vector3x8::MulAndAdd(Vector3x8(mUpScaled), offsetedCoords.y, direction);

    // depth of field
    if (mDOF.aperture > 0.001f)
    {
        const Vector3x8 focusPoint = origin + direction * mDOF.focalPlaneDistance;

        // TODO different bokeh shapes, texture, etc.
        const Vector2x8 randomPointOnCircle = context.randomGenerator.GetCircle_Simd8() * mDOF.aperture;
        origin = Vector3x8::MulAndAdd(Vector3x8(mRightInternal), randomPointOnCircle.x, origin);
        origin = Vector3x8::MulAndAdd(Vector3x8(mUpInternal), randomPointOnCircle.y, origin);

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

    return Vector4();
}

} // namespace rt
