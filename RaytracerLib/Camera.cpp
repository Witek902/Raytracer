#include "PCH.h"
#include "Camera.h"


namespace rt {

using namespace math;

Camera::Camera()
    : mAspectRatio(1.0f)
    , mFieldOfView(RT_PI * 70.0f / 180.0f)
    , mMode(CameraMode::Perspective)
{ }

void Camera::SetPerspective(const math::Vector4& pos, const math::Vector4& dir, const math::Vector4& up, Float aspectRatio, Float FoV)
{
    mMode = CameraMode::Perspective;
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

    // field of view
    const Float tanHalfFoV = tanf(mFieldOfView * 0.5f);
    mUpInternal *= tanHalfFoV;
    mRightInternal *= tanHalfFoV;

    // aspect ratio
    mRightInternal *= mAspectRatio;
}

math::Ray Camera::GenerateRay(const math::Vector4& coords) const
{
    Vector4 origin = mPosition;
    Vector4 direction;

    switch (mMode)
    {
        case CameraMode::Perspective:
        {
            const Vector4 offsetedCoords = 2.0f * coords - VECTOR_ONE;
            direction = mForwardInternal + offsetedCoords[0] * mRightInternal + offsetedCoords[1] * mUpInternal;
            break;
        }

        // TODO more types:
        // ortho, fisheye, spherical, etc.
    }

    return Ray(origin, direction);
}


} // namespace rt
