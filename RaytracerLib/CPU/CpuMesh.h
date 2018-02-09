#pragma once

#include "../Mesh.h"
#include "Math/Box.h"
#include "Math/Ray.h"
#include "Math/Simd4Ray.h"
#include "Math/Simd8Ray.h"

#include "VertexBuffer.h"
#include "../Material.h"
#include "../BVH.h"

namespace rt {


// Preliminary ray-mesh intersection data (non-SIMD)
struct MeshIntersectionData
{
    float distance;
    float u;
    float v;
    Uint32 triangle;
};

// Preliminary ray-mesh intersection data (4-ray SIMD version)
struct MeshIntersectionData_Simd4
{
    math::Vector4 distance;
    math::Vector4 u;
    math::Vector4 v;
    math::Vector4 triangle;
};

// Preliminary ray-mesh intersection data (8-ray SIMD version)
struct MeshIntersectionData_Simd8
{
    math::Vector8 distance;
    math::Vector8 u;
    math::Vector8 v;
    math::Vector8 triangle;
};

struct ShadingData
{
    const Material* material;
    math::Vector4 position;
    math::Vector4 normal;
    math::Vector4 tangent;
    math::Vector4 binormal;
    math::Vector4 texCoord;
};

/**
 * Class representing a mesh.
 */
class RT_ALIGN(16) CpuMesh : public IMesh
{
public:
    CpuMesh();
    ~CpuMesh();

    // Initialize the mesh
    virtual bool Initialize(const MeshDesc& desc) override;

    // Trace the mesh (non-SIMD version) to obtain preliminary intersection data
    // 'maxDistance' allows for narrowing of intersection search range
    // Returns true if an intersection occurred
    bool RayTrace_Single(const math::Ray& ray, float maxDistance, MeshIntersectionData& outData) const;

    // Trace the mesh against 4 rays (SIMD version) to obtain preliminary intersection data
    // 'maxDistance' allows for narrowing of intersection search range
    // Returns bitmask telling if an intersection occurred for each ray
    Uint8 RayTrace_Simd4(const math::Ray_Simd4& rays, const math::Vector4& maxDistances, MeshIntersectionData_Simd4& outData) const;

    // Trace the mesh against 8 rays (SIMD version) to obtain preliminary intersection data
    // 'maxDistance' allows for narrowing of intersection search range
    // Returns bitmask telling if an intersection occurred for each ray
    Uint8 RayTrace_Simd8(const math::Ray_Simd8& rays, const math::Vector8& maxDistances, MeshIntersectionData_Simd8& outData) const;

    // Calculate input data for shading routine
    void EvaluateShadingData_Single(const math::Ray& ray, const MeshIntersectionData& intersectionData,
                                    ShadingData& outShadingData) const;

private:

    // bounding box after scaling
    math::Box mBoundingBox;

    // vertex data
    VertexBuffer mVertexBuffer;

    // bounding volume hierarchy for tracing acceleration
    BVH mBVH;

    std::string mPath;
};


} // namespace rt
