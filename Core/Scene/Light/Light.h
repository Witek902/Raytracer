#pragma once

#include "../../RayLib.h"
#include "../../Math/Box.h"
#include "../../Math/Matrix4.h"
#include "../../Color/RayColor.h"
#include "../../Color/Spectrum.h"
#include "../../Utils/AlignmentAllocator.h"

namespace rt {

namespace math {
class Ray;
} // math

struct RenderingContext;
struct IntersectionData;
struct HitPoint;

// abstract light
class RT_ALIGN(16) ILight : public Aligned<16>
{
public:
    static constexpr const float BackgroundLightDistance = std::numeric_limits<float>::max();
    static constexpr const float CosEpsilon = 0.9999f;

    enum class Type : Uint8
    {
        Area,
        Background,
        Directional,
        Point,
        Spot,
    };

    enum Flags : Uint8
    {
        Flag_None       = 0,
        Flag_IsFinite   = 1 << 0,   // light has finite extent (e.g. point or area light)
        Flag_IsDelta    = 1 << 1,   // light cannot be hit by camera ray directly (e.g. directional light or point light)
    };

    struct RadianceParam
    {
        RenderingContext& context;
        const math::Ray& ray;
        const math::Vector4 hitPoint = math::Vector4::Zero();
        const float cosAtLight = 1.0f;
        bool rendererSupportsSolidAngleSampling = true;
    };

    struct IlluminateParam
    {
        const math::Matrix4 worldToLight;       // transform from world space to light local space
        const math::Matrix4 lightToWorld;       // transform from light local space to world space
        const IntersectionData& intersection;   // intersection data of the shaded object
        Wavelength& wavelength;
        math::Float3 sample;
        bool rendererSupportsSolidAngleSampling = true;
    };

    struct IlluminateResult
    {
        math::Vector4 directionToLight = math::Vector4::Zero();
        float distance = -1.0f;
        float directPdfW = -1.0f;
        float emissionPdfW = -1.0f;
        float cosAtLight = -1.0f;
    };

    struct EmitParam
    {
        const math::Matrix4 lightToWorld; // transform from light local space to world space
        Wavelength& wavelength;
        math::Float3 positionSample;
        math::Float2 directionSample;
    };

    struct EmitResult
    {
        math::Vector4 position;
        math::Vector4 direction;
        float directPdfA;
        float emissionPdfW;
        float cosAtLight;
    };

    explicit ILight(const math::Vector4& color = math::Vector4(1.0f));
    RAYLIB_API virtual ~ILight() = default;

    RT_FORCE_INLINE const Spectrum& GetColor() const { return mColor; }
    RAYLIB_API void SetColor(const Spectrum& color);

    // get light's type
    virtual Type GetType() const = 0;

    // get light's surface bounding box
    virtual const math::Box GetBoundingBox() const = 0;

    // check if a ray hits the light
    virtual bool TestRayHit(const math::Ray& ray, float& outDistance) const = 0;

    // Illuminate a point in the scene.
    // Returns probability of sampling the returned direction.
    virtual const RayColor Illuminate(const IlluminateParam& param, IlluminateResult& outResult) const = 0;

    // Emit random light photon from the light
    virtual const RayColor Emit(const EmitParam& param, EmitResult& outResult) const = 0;

    // Returns radiance for ray hitting the light directly
    // Optionally returns probability of hitting this point and emitting a photon in that direction
    virtual const RayColor GetRadiance(const RadianceParam& param, float* outDirectPdfA = nullptr, float* outEmissionPdfW = nullptr) const;

    // Get light flags.
    virtual Flags GetFlags() const = 0;

private:
    // light object cannot be copied
    ILight(const ILight&) = delete;
    ILight& operator = (const ILight&) = delete;

    Spectrum mColor;
};

using LightPtr = std::unique_ptr<ILight>;

} // namespace rt
