#pragma once

#include "RayLib.h"
#include "Math/Vector4.h"
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

enum class BokehType
{
    Circle,
    Hexagon,
};

/**
 * Depth of Field settings.
 */
struct DOFSettings
{
    Float focalPlaneDistance;
    Float aperture;
    BokehType bokehType;

    DOFSettings()
        : focalPlaneDistance(10.0f)
        , aperture(0.1f)
        , bokehType(BokehType::Circle)
    { }
};


/**
 * Class describing camera for scene raytracing.
 */
RT_ALIGN(16)
class RAYLIB_API Camera
{
public:
    Camera();

    void SetPerspective(const math::Vector4& pos, const math::Vector4& dir, const math::Vector4& up, Float aspectRatio, Float FoV);

    // Update internal state
    // Should be called once before rendering
    void Update();

    // Generate ray for the camera
    // x and y coordinates should be in [0.0f, 1.0f) range.
    math::Ray GenerateRay(const math::Vector4& coords) const;

    // TODO generate ray packet

    math::Vector4 mUp;
    Float mAspectRatio;         // width / height
    Float mFieldOfView;         // in radians, vertical angle

    CameraMode mMode;

    DOFSettings mDOF;

    // TODO more advanced effects:
    // - chromatic aberration
    // - barrel distortion
    // - vignetting
    // etc...

    math::Vector4 mPosition;
    math::Vector4 mForward;

private:
    math::Vector4 mForwardInternal;
    math::Vector4 mRightInternal;
    math::Vector4 mUpInternal;
};

} // namespace rt