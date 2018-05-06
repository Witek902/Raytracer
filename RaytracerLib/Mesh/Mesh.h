#pragma once

#include "../RayLib.h"

#include "VertexBuffer.h"

#include "../Traversal/HitPoint.h"
#include "../BVH/BVH.h"

#include "../Math/Box.h"
#include "../Math/Ray.h"
#include "../Math/Simd4Ray.h"
#include "../Math/Simd8Ray.h"


namespace rt {

struct LocalCounters;

struct ShadingData
{
    const Material* material = nullptr;

    math::Vector4 position;

    math::Vector4 tangent;
    math::Vector4 bitangent;
    math::Vector4 normal;

    math::Vector4 texCoord;

    math::Vector4 LocalToWorld(const math::Vector4 localCoords) const;
    math::Vector4 WorldToLocal(const math::Vector4 worldCoords) const;
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

    RT_FORCE_INLINE const math::Box& GetBoundingBox() const { return mBoundingBox; }

    // Trace the mesh to obtain intersection data (aka. hit point)
    // Note: 'distance' is used for narrowing of intersection search range
    void Traverse_Single(const math::Ray& ray, HitPoint& hitPoint) const;
    void Traverse_Simd8(const math::Ray_Simd8& ray, HitPoint_Simd8& hitPoint, Uint32 instanceID) const;
    void Traverse_Packet(const RayPacket& packet, HitPoint_Packet& data, Uint32 instanceID) const;

    // Calculate input data for shading routine
    void EvaluateShadingData_Single(const HitPoint& hitPoint, ShadingData& outShadingData) const;

private:

    void Traverse_Leaf_Single(const math::Ray& ray, const BVH::Node& node, HitPoint& outHitPoint) const;
    void Traverse_Leaf_Simd8(const math::Ray_Simd8& ray, const BVH::Node& node, const Uint32 instanceID, HitPoint_Simd8& outHitPoint) const;

    // bounding box after scaling
    math::Box mBoundingBox;

    // vertex data
    VertexBuffer mVertexBuffer;

    // bounding volume hierarchy for tracing acceleration
    BVH mBVH;

    std::string mPath;
};


} // namespace rt
