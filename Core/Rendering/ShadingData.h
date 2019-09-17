#pragma once

#include "../Common.h"
#include "../Color/RayColor.h"
#include "../Math/Matrix4.h"
#include "../Traversal/Intersection.h"

namespace rt {

class Material;

struct SampledMaterialParameters
{
    RayColor baseColor;
    RayColor emissionColor;
    float roughness;
    float metalness;
    float IoR;
};

struct ShadingData
{
    // geometry data
    IntersectionData intersection;

    // incoming ray data
    math::Vector4 outgoingDirWorldSpace;

    SampledMaterialParameters materialParams;
};

} // namespace rt
