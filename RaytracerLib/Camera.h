#pragma once

#include "RayLib.h"
#include "Math/Vector.h"
#include "Math/Matrix.h"
#include "Math/Ray.h"

namespace rt {


enum class CameraMode
{
    Perspective,

    // TODO:
    // Orthogonal,
    // Spherical,
    // Fisheye,
};


/**
 * Class describing camera for scene raytracing.
 */
RT_ALIGN(16)
class RAYLIB_API Camera
{
public:
    Camera();

    void SetPerspective(const math::Vector& pos, const math::Vector& dir, const math::Vector& up, Float aspectRatio, Float FoV);

    // Update internal state
    // Should be called once before rendering
    void Update();

    // Generate ray for the camera
    // x and y coordinates should be in [0.0f, 1.0f) range.
    math::Ray GenerateRay(Float x, Float y) const;

    // TODO generate ray packet

    math::Vector mUp;
    Float mAspectRatio;         // width / height
    Float mFieldOfView;         // in radians, vertical angle

    CameraMode mMode;

    math::Vector mPosition;
    math::Vector mForward;

private:
    math::Vector mForwardInternal;
    math::Vector mRightInternal;
    math::Vector mUpInternal;
};

} // namespace rt