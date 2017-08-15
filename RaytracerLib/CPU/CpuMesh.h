#pragma once

#include "../Mesh.h"
#include "Math/Box.h"
#include "Math/Ray.h"

#include "VertexBuffer.h"
#include "../Material.h"

namespace rt {


// Preliminary ray-mesh intersection data
struct MeshIntersectionData
{
    float distance;
    float u;
    float v;
    Uint32 triangle;
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
    bool RayTrace_Single(const math::Ray& ray, float maxDistance, MeshIntersectionData& outData) const;

    // Calculate input data for shading routine
    void EvaluateShadingData_Single(const math::Ray& ray, const MeshIntersectionData& intersectionData,
                                    ShadingData& outShadingData) const;

private:

    // bounding box after scaling
    math::Box mBoundingBox;

    // vertex data
    VertexBuffer mVertexBuffer;

    // Scaling factor. Used when vertex data format is non-float
    math::Vector4 mScale;

    const char* mDebugName;

    // TODO BVH
};


} // namespace rt
