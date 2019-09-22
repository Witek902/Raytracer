#pragma once

#include "Shape.h"
#include "Mesh/VertexBuffer.h"

#include "../Traversal/HitPoint.h"
#include "../BVH/BVH.h"

#include "../Math/Box.h"
#include "../Math/Ray.h"
#include "../Math/Simd8Ray.h"


namespace rt {

struct IntersectionData;
struct SingleTraversalContext;
struct PacketTraversalContext;

struct MeshDesc
{
    VertexBufferDesc vertexBufferDesc;
    std::string path;
};

class RT_ALIGN(16) MeshShape : public IShape
{
public:
    RAYLIB_API MeshShape();
    RAYLIB_API ~MeshShape();

    // Initialize the mesh
    RAYLIB_API bool Initialize(const MeshDesc& desc);

    // IShape
    virtual const math::Box GetBoundingBox() const override;
    virtual float GetSurfaceArea() const override;
    virtual void Traverse(const SingleTraversalContext& context, const uint32 objectID) const override;
    virtual bool Traverse_Shadow(const SingleTraversalContext& context) const override;
    virtual const math::Vector4 Sample(const math::Float3& u, math::Vector4 * outNormal, float* outPdf = nullptr) const override;
    virtual void EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outIntersectionData) const override;

    RT_FORCE_INLINE const BVH& GetBVH() const { return mBVH; }

    // Intersect ray(s) with BVH leaf
    void Traverse_Leaf(const SingleTraversalContext& context, const uint32 objectID, const BVH::Node& node) const;
    void Traverse_Leaf(const PacketTraversalContext& context, const uint32 objectID, const BVH::Node& node, const uint32 numActiveGroups) const;

    // Intersect shadow ray(s) with BVH leaf
    // Returns true if any hit was found
    bool Traverse_Leaf_Shadow(const SingleTraversalContext& context, const BVH::Node& node) const;

private:

    // bounding box after scaling
    math::Box mBoundingBox;

    // vertex data
    VertexBuffer mVertexBuffer;

    // bounding volume hierarchy for tracing acceleration
    BVH mBVH;

    std::string mPath;
};

using MeshShapePtr = std::shared_ptr<MeshShape>;

} // namespace rt
