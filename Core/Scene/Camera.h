#pragma once

#include "../RayLib.h"

#include "../Math/Transform.h"
#include "../Math/Ray.h"
#include "../Math/Random.h"
#include "../Math/Simd8Ray.h"
#include "../Math/Matrix4.h"

namespace rt {

namespace math {
class Vector2x8;
}

struct RenderingContext;

enum class BokehShape : Uint8
{
    Circle = 0,
    Hexagon,
    Square,
    NGon,
};

/**
 * Depth of Field settings.
 */
struct DOFSettings
{
    // distance from camera at which plane of perfect focus is located
    Float focalPlaneDistance = 2.0f;

    // the bigger value, the bigger out-of-focus blur
    Float aperture = 0.1f;

    bool enable = false;

    BokehShape bokehType = BokehShape::Circle;

    // used when bokeh type is "NGon"
    Uint32 apertureBlades = 5;
};


/**
 * Class describing camera for scene raytracing.
 */
class RT_ALIGN(16) Camera
{
public:
    RAYLIB_API Camera();

    RAYLIB_API void SetTransform(const math::Transform& transform);

    RAYLIB_API void SetPerspective(Float aspectRatio, Float FoV);

    RAYLIB_API void SetAngularVelocity(const math::Quaternion& quat);

    RT_FORCE_INLINE const math::Transform& GetTransform() const { return mTransform; }
    RT_FORCE_INLINE const math::Matrix4& GetLocalToWorld() const { return mLocalToWorld; }

    // Sample camera transfrom for given time point
    RT_FORCE_INLINE const math::Matrix4 SampleTransform(const float time) const;

    // Generate ray for the camera for a given time
    // x and y coordinates should be in [0.0f, 1.0f) range.
    RAYLIB_API RT_FORCE_NOINLINE math::Ray GenerateRay(const math::Vector4 coords, RenderingContext& context) const;
    RT_FORCE_NOINLINE math::Ray_Simd8 GenerateRay_Simd8(const math::Vector2x8& coords, RenderingContext& context) const;

    RT_FORCE_INLINE const math::Vector4 GenerateBokeh(RenderingContext& context) const;
    RT_FORCE_INLINE const math::Vector2x8 GenerateBokeh_Simd8(RenderingContext& context) const;

    // Convert world-space coordinates to film-space coordinates including camera projection (0...1 range)
    bool WorldToFilm(const math::Vector4 worldPosition, math::Vector4& outFilmCoords) const;

    Float PdfW(const math::Vector4 direction) const;

    // camera placement
    math::Transform mTransform;

    // width to height ratio
    Float mAspectRatio;

    // in radians, vertical angle
    Float mFieldOfView;

    // depth of field settings
    DOFSettings mDOF;

    // camera lens distortion (0.0 - no distortion)
    Float barrelDistortionConstFactor;
    Float barrelDistortionVariableFactor;
    bool enableBarellDistortion;

private:
    Float mTanHalfFoV;

    math::Matrix4 mLocalToWorld;
    math::Matrix4 mWorldToScreen;
};

} // namespace rt