#pragma once

#include "../../RayLib.h"
#include "../../Math/Box.h"
#include "../../Color/RayColor.h"
#include "../../Color/Spectrum.h"
#include "../../Utils/AlignmentAllocator.h"

namespace rt {

namespace math {
class Ray;
} // math

struct RenderingContext;
struct ShadingData;

// abstract light
class RT_ALIGN(16) ILight : public Aligned<16>
{
public:
    static constexpr Float BackgroundLightDistance = 1.0e+36f;

    struct IlluminateParam
    {
        const ShadingData& shadingData;
        RenderingContext& context;

        math::Vector4 outDirectionToLight = math::Vector4::Zero();
        float outDistance = 0.0f;
        float outDirectPdfW = 0.0f;
    };

    RAYLIB_API ILight(const math::Vector4 color);
    RAYLIB_API virtual ~ILight() = default;

    // get light's surface bounding box
    virtual const math::Box GetBoundingBox() const = 0;

    // check if a ray hits the light
    virtual bool TestRayHit(const math::Ray& ray, Float& outDistance) const = 0;

    // Illuminate a point in the scene.
    // Returns probability of sampling the returned direction.
    virtual const RayColor Illuminate(IlluminateParam& param) const = 0;

    // get normal vector at intersection point
    virtual const math::Vector4 GetNormal(const math::Vector4& hitPoint) const;

    /*
    // Emit random light photon from the light
    virtual math::Vector4 Emit(
        math::Vector4& outPosition,
        math::Vector4& outDirection,
        float& outPdfW) const = 0;
        */

    // Returns radiance for ray hitting the light directly
    // Optionally returns probability of hitting this point
    virtual const RayColor GetRadiance(
        RenderingContext& context,
        const math::Vector4& rayDirection,
        const math::Vector4& hitPoint,
        Float* outDirectPdfA = nullptr) const;

    // Returs true if the light has finite extent.
    // E.g. point or area light.
    virtual bool IsFinite() const = 0;

    // Returns true if the light cannot be hit by camera ray directly.
    // E.g. directional light or point light.
    virtual bool IsDelta() const = 0;

protected:

    Spectrum mColor;

private:
    // light object cannot be copied
    ILight(const ILight&) = delete;
    ILight& operator = (const ILight&) = delete;
};


} // namespace rt
