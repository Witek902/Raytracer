#pragma once

#include "RayLib.h"
#include "Math/Vector4.h"
#include "Math/Matrix.h"
#include "Math/Ray.h"
#include "Math/Random.h"

namespace rt {


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

    // the bigger value, the bigger out-of-focus blur
    Float aperture;

    BokehType bokehType;

    DOFSettings()
        : focalPlaneDistance(1.0f)
        , aperture(0.02f)
        , bokehType(BokehType::Circle)
    { }
};


/**
 * Class describing camera for scene raytracing.
 */
class RT_ALIGN(16) RAYLIB_API Camera
{
public:
    Camera();

    void SetPerspective(const math::Vector4& pos, const math::Vector4& dir, const math::Vector4& up, Float aspectRatio, Float FoV);

    // Update internal state
    // Should be called once before rendering
    void Update();

    // Generate ray for the camera
    // x and y coordinates should be in [0.0f, 1.0f) range.
    math::Ray GenerateRay(const math::Vector4& coords, math::Random& randomGenerator) const;

    // TODO generate ray packet

    // camera placement
    math::Vector4 mPosition;
    math::Vector4 mForward;
    math::Vector4 mUp;

    // width to height ratio
    Float mAspectRatio;

    // in radians, vertical angle
    Float mFieldOfView;

    // depth of field settings
    DOFSettings mDOF;

    // camera lens distortion (0.0 - no distortion)
    Float barrelDistortionFactor;

private:
    math::Vector4 mForwardInternal;
    math::Vector4 mRightInternal;
    math::Vector4 mUpInternal;
    math::Vector4 mRightScaled;
    math::Vector4 mUpScaled;
};

} // namespace rt