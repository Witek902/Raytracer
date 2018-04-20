#pragma once

#include "../RayLib.h"

#include "VertexBuffer.h"

#include "../Traversal/IntersectionData.h"
#include "../Material/Material.h"
#include "../BVH/BVH.h"

#include "../Math/Box.h"
#include "../Math/Ray.h"
#include "../Math/Simd4Ray.h"
#include "../Math/Simd8Ray.h"


namespace rt {

struct LocalCounters;

struct ShadingData
{
    const Material* material;
    math::Vector4 position;
    math::Vector4 normal;
    math::Vector4 tangent;
    math::Vector4 binormal;
    math::Vector4 texCoord;
};


class Material;

struct MeshDesc
{
    VertexBufferDesc vertexBufferDesc;
    std::string path;
};


/**
 * Class representing a mesh.
 */
class RAYLIB_API RT_ALIGN(16) Mesh
{
public:
    Mesh();
    ~Mesh();

    // Initialize the mesh
    bool Initialize(const MeshDesc& desc);

    // Trace the mesh (non-SIMD version) to obtain intersection data (aka. hit point)
    // Note: 'distance' is used for narrowing of intersection search range
    void RayTrace_Single(const math::Ray& ray, RayIntersectionData& data, LocalCounters& counters) const;

    // Calculate input data for shading routine
    void EvaluateShadingData_Single(const math::Ray& ray, const RayIntersectionData& intersectionData, ShadingData& outShadingData) const;

    // Trace the mesh (non-SIMD version) to obtain intersection data (aka. hit point)
    // Note: 'distance' is used for narrowing of intersection search range
    void RayTrace_Packet(const RayPacket& packet, RayPacketIntersectionData& data, Uint32 instanceID) const;

private:

    void RayTrace_Leaf_Single(const math::Ray& ray, const BVH::Node& node, RayIntersectionData& outData, LocalCounters& counters) const;

    // bounding box after scaling
    math::Box mBoundingBox;

    // vertex data
    VertexBuffer mVertexBuffer;

    // bounding volume hierarchy for tracing acceleration
    BVH mBVH;

    std::string mPath;
};


} // namespace rt
