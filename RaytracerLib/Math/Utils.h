#pragma once

namespace rt {
namespace math {


// compute Fresnel reflection term for dielectric material
float FresnelDielectric(float NdV, float eta, bool& totalInternalReflection);

// compute Fresnel reflection term for metalic material
float FresnelMetal(const float NdV, const float eta, const float k);


} // namespace math
} // namespace rt
