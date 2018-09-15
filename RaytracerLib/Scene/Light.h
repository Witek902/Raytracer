#pragma once

#include "../RayLib.h"
#include "../Math/Vector4.h"


namespace rt {


// abstract light
class ILight
{
public:
    virtual ~ILight() = default;

    // Illuminate a point in the scene.
    virtual math::Vector4 Illuminate(
        const math::Vector4& point,
        math::Vector4& outDirectionToLight,
        float& outDistance,
        float& outPdfW,
        float* outEmissionPdfW = nullptr,
        float* outCosAtLight = nullptr) const = 0;

    // Emit random light photon from the light
    virtual math::Vector4 Emit(
        math::Vector4& outPosition,
        math::Vector4& outDirection,
        float& outPdfW) const = 0;

    // Returns radiance for ray hitting the light directly
    virtual math::Vector4 GetRadiance(
        const math::Vector4& rayDirection,
        const math::Vector4& hitPoint,
        float* outDirectPdfA = nullptr,
        float* outEmissionPdfW = nullptr) const = 0;

    // Returs true if the light has finite extent.
    // E.g. point or area light.
    virtual bool IsFinite() const = 0;

    // Returns true if the light cannot be hit by camera ray directly.
    // E.g. directional light or point light.
    virtual bool IsDelta() const = 0;
};


} // namespace rt
