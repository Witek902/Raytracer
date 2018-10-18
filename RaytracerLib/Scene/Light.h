#pragma once

#include "../RayLib.h"
#include "../Math/Box.h"
#include "../Math/Ray.h"
#include "../Color/Color.h"
#include "../Utils/AlignmentAllocator.h"

namespace rt {

struct RenderingContext;

// abstract light
class RT_ALIGN(16) RAYLIB_API ILight : public Aligned<16>
{
public:
    ILight() = default;
    virtual ~ILight() = default;

    // get light's surface bounding box
    virtual const math::Box GetBoundingBox() const = 0;

    // check if a ray hits the light
    virtual bool TestRayHit(const math::Ray& ray, Float& outDistance) const = 0;

    // Illuminate a point in the scene.
    // Returns probability of sampling the returned direction.
    virtual const Color Illuminate(
        const math::Vector4& scenePoint,
        RenderingContext& context,
        math::Vector4& outDirectionToLight,
        float& outDistance,
        float& outDirectPdfW) const = 0;

    /*
    // Emit random light photon from the light
    virtual math::Vector4 Emit(
        math::Vector4& outPosition,
        math::Vector4& outDirection,
        float& outPdfW) const = 0;
        */

    // Returns radiance for ray hitting the light directly
    // Optionally returns probability of hitting this point
    virtual const Color GetRadiance(
        RenderingContext& context,
        const math::Vector4& rayDirection,
        const math::Vector4& hitPoint,
        Float* outDirectPdfA = nullptr) const = 0;

    // Returs true if the light has finite extent.
    // E.g. point or area light.
    virtual bool IsFinite() const = 0;

    // Returns true if the light cannot be hit by camera ray directly.
    // E.g. directional light or point light.
    virtual bool IsDelta() const = 0;

private:
    // light object cannot be copied
    ILight(const ILight&) = delete;
    ILight& operator = (const ILight&) = delete;
};


/////////////////////////////////////////////////////////////////

class RAYLIB_API PointLight : public ILight
{
public:
    PointLight() = default;

    PointLight(const math::Vector4& position, const math::Vector4& color)
        : position(position)
        , color(color)
    {}

    math::Vector4 position;
    math::Vector4 color;

    virtual const math::Box GetBoundingBox() const override;
    virtual bool TestRayHit(const math::Ray& ray, Float& outDistance) const override;
    virtual const Color Illuminate(const math::Vector4& scenePoint, RenderingContext& context, math::Vector4& outDirectionToLight, float& outDistance, float& outDirectPdfW) const override;
    virtual const Color GetRadiance(RenderingContext& context, const math::Vector4& rayDirection, const math::Vector4& hitPoint, Float* outDirectPdfA) const override;
    virtual bool IsFinite() const override final;
    virtual bool IsDelta() const override final; 
};


/////////////////////////////////////////////////////////////////

class RAYLIB_API AreaLight : public ILight
{
public:
    AreaLight(const math::Vector4& p0, const math::Vector4& edge0, const math::Vector4& edge1, const math::Vector4& color);

    virtual const math::Box GetBoundingBox() const override;
    virtual bool TestRayHit(const math::Ray& ray, Float& outDistance) const override;
    virtual const Color Illuminate(const math::Vector4& scenePoint, RenderingContext& context, math::Vector4& outDirectionToLight, float& outDistance, float& outDirectPdfW) const override;
    virtual const Color GetRadiance(RenderingContext& context, const math::Vector4& rayDirection, const math::Vector4& hitPoint, Float* outDirectPdfA) const override;
    virtual bool IsFinite() const override final;
    virtual bool IsDelta() const override final;

private:
    math::Vector4 p0;
    math::Vector4 edge0;
    math::Vector4 edge1;

    math::Vector4 color;

    Float invArea; // inverted surface area

    bool isTriangle = false;
};


/////////////////////////////////////////////////////////////////

class RAYLIB_API DirectionalLight : public ILight
{
public:
    DirectionalLight() = default;

    DirectionalLight(const math::Vector4& direction, const math::Vector4& color)
        : direction(direction.Normalized3())
        , color(color)
    {}

    math::Vector4 direction; // incident light direction
    math::Vector4 color;

    virtual const math::Box GetBoundingBox() const override;
    virtual bool TestRayHit(const math::Ray& ray, Float& outDistance) const override;
    virtual const Color Illuminate(const math::Vector4& scenePoint, RenderingContext& context, math::Vector4& outDirectionToLight, float& outDistance, float& outDirectPdfW) const override;
    virtual const Color GetRadiance(RenderingContext& context, const math::Vector4& rayDirection, const math::Vector4& hitPoint, Float* outDirectPdfA) const override;
    virtual bool IsFinite() const override final;
    virtual bool IsDelta() const override final;
};

// TODO background light

// TODO directional light

// TODO spherical light (solid angle sampling)

} // namespace rt
