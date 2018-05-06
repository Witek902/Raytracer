#pragma once

#include "../RayLib.h"

#include "../Math/Vector4.h"
#include "../Math/Matrix.h"
#include "../Math/Ray.h"
#include "../Math/Random.h"

namespace rt {

struct RenderingContext;

enum class BokehShape : Uint8
{
    Circle,
    Hexagon,
    Square,
};

/**
 * Depth of Field settings.
 */
struct DOFSettings
{
    // distance from camera at which plane of perfect focus is located
    Float focalPlaneDistance;

    // the bigger value, the bigger out-of-focus blur
    Float aperture;

    BokehShape bokehType;

    DOFSettings()
        : focalPlaneDistance(2.0f)
        , aperture(0.002f)
        , bokehType(BokehShape::Circle)
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

    // Generate ray for the camera for a given time
    // x and y coordinates should be in [0.0f, 1.0f) range.
    math::Ray GenerateRay(const math::Vector4 coords, RenderingContext& context) const;

    // TODO generate ray packet

    // camera placement
    math::Vector4 mPosition;
    math::Vector4 mForward;
    math::Vector4 mUp;

    math::Vector4 mPositionDelta;

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