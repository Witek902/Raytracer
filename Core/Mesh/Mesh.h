#pragma once

#include "../RayLib.h"

#include "VertexBuffer.h"

#include "../Traversal/HitPoint.h"
#include "../BVH/BVH.h"

#include "../Math/Box.h"
#include "../Math/Ray.h"
#include "../Math/Simd8Ray.h"


namespace rt {

struct ShadingData;
struct SingleTraversalContext;
struct PacketTraversalContext;

struct MeshDesc
{
    VertexBufferDesc vertexBufferDesc;
    std::string path;
};


/**
 * Class representing a mesh.
 */
class RT_ALIGN(16) Mesh : public Aligned<16>
{
public:
    RAYLIB_API Mesh();
    RAYLIB_API ~Mesh();

    // Initialize the mesh
    RAYLIB_API bool Initialize(const MeshDesc& desc);

    RT_FORCE_INLINE const math::Box& GetBoundingBox() const { return mBoundingBox; }
    RT_FORCE_INLINE const BVH& GetBVH() const { return mBVH; }

    // Intersect ray(s) with BVH leaf
    void Traverse_Leaf_Single(const SingleTraversalContext& context, const Uint32 objectID, const BVH::Node& node) const;
    void Traverse_Leaf_Packet(const PacketTraversalContext& context, const Uint32 objectID, const BVH::Node& node, const Uint32 numActiveGroups) const;

    // Intersect shadow ray(s) with BVH leaf
    // Returns true if any hit was found
    bool Traverse_Leaf_Shadow_Single(const SingleTraversalContext& context, const BVH::Node& node) const;

    // Calculate input data for shading routine
    void EvaluateShadingData_Single(const HitPoint& hitPoint, ShadingData& outShadingData, const Material* defaultMaterial) const;

private:

    // bounding box after scaling
    math::Box mBoundingBox;

    // vertex data
    VertexBuffer mVertexBuffer;

    // bounding volume hierarchy for tracing acceleration
    BVH mBVH;

    std::string mPath;
};

using MeshPtr = std::shared_ptr<Mesh>;

} // namespace rt
