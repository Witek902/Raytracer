#include "PCH.h"

#include "Mesh.h"
#include "Material/Material.h"

#include "BVH/BVHBuilder.h"

#include "Traversal/HitPoint.h"
#include "Rendering/Counters.h"

#include "Math/Triangle.h"
#include "Math/Geometry.h"
#include "Math/Simd4Geometry.h"
#include "Math/Simd8Geometry.h"

#include "Utils/Logger.h"
#include "Utils/Logger.h"
#include "Utils/AlignmentAllocator.h"

#include <vector>
#include <sstream>


namespace rt {

using namespace math;

const static Material gDefaultMaterial;

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

bool Mesh::Initialize(const MeshDesc& desc)
{
    if (!mVertexBuffer.Initialize(desc.vertexBufferDesc))
    {
        RT_LOG_ERROR("Failed to initialize vertex buffer");
        return false;
    }

    std::vector<Box, AlignmentAllocator<Box>> boxes;

    const Uint32 numTriangles = mVertexBuffer.GetNumTriangles();
    for (Uint32 i = 0; i < numTriangles; ++i)
    {
        VertexIndices indices;
        mVertexBuffer.GetVertexIndices(i, indices);

        Triangle tri;
        mVertexBuffer.GetVertexPositions(indices, tri);


        Box triBox(tri.v0, tri.v1, tri.v2);
        triBox.min -= VECTOR_EPSILON;
        triBox.max += VECTOR_EPSILON;

        boxes.push_back(triBox);
    }

    BVHBuilder::BuildingParams params;
    params.maxLeafNodeSize = 2;

    BVHBuilder::Indices newTrianglesOrder;
    BVHBuilder bvhBuilder(mBVH);
    if (!bvhBuilder.Build(boxes.data(), numTriangles, params, newTrianglesOrder))
    {
        return false;
    }

    // calculate & print stats
    {
        BVH::Stats stats;
        mBVH.CalculateStats(stats);
        RT_LOG_INFO("BVH stats:");
        RT_LOG_INFO("    - max depth: %u", stats.maxDepth);
        RT_LOG_INFO("    - total surface area: %f", stats.totalNodesArea);
        RT_LOG_INFO("    - total volume: %f", stats.totalNodesVolume);

        std::stringstream str;
        for (size_t i = 0; i < stats.leavesCountHistogram.size(); ++i)
        {
            if (i > 0)
                str << ", ";
            str << i << " (" << stats.leavesCountHistogram[i] << ")";
        }
        RT_LOG_INFO("    - leaf nodes histogram: %s", str.str().c_str());
    }

    const std::string bvhCachePath = desc.path + ".bvhcache";
    mBVH.SaveToFile(bvhCachePath);

    mVertexBuffer.ReorderTriangles(newTrianglesOrder);

    // TODO reorder indices

    RT_LOG_INFO("Mesh '%s' created successfully", !desc.path.empty() ? desc.path.c_str() : "unnamed");
    return true;
}

void Mesh::Traverse_Leaf_Single(const math::Ray& ray, const BVH::Node& node, HitPoint& hitPoint) const
{
    float distance, u, v;

    for (Uint32 i = 0; i < node.data.numLeaves; ++i)
    {
        const Uint32 triangleIndex = node.data.childIndex + i;

        VertexIndices indices;
        mVertexBuffer.GetVertexIndices(triangleIndex, indices);

        Triangle tri;
        mVertexBuffer.GetVertexPositions(indices, tri);

        if (Intersect_TriangleRay(ray, tri, u, v, distance))
        {
            if (distance < hitPoint.distance)
            {
                hitPoint.distance = distance;
                hitPoint.triangleId = triangleIndex;
                hitPoint.u = u;
                hitPoint.v = v;
            }
        }
    }
}

void Mesh::Traverse_Leaf_Simd8(const math::Ray_Simd8& ray, const BVH::Node& node, const Uint32 instanceID, HitPoint_Simd8& outHitPoint) const
{
    const Vector8 vInstanceID = Vector8::Splat(instanceID);

    Vector8 distance, u, v;

    // TODO counters

    for (Uint32 i = 0; i < node.data.numLeaves; ++i)
    {
        const Uint32 triangleIndex = node.data.childIndex + i;

        VertexIndices indices;
        mVertexBuffer.GetVertexIndices(triangleIndex, indices);

        Triangle tri;
        mVertexBuffer.GetVertexPositions(indices, tri);

        const Triangle_Simd8 simdTri(tri);

        const Vector8 mask = Intersect_TriangleRay_Simd8(ray, simdTri, outHitPoint.distance, u, v, distance);

        if (mask.GetSignMask())
        {
            // combine results according to mask
            outHitPoint.u = Vector8::SelectBySign(outHitPoint.u, u, mask);
            outHitPoint.v = Vector8::SelectBySign(outHitPoint.v, v, mask);
            outHitPoint.distance = Vector8::SelectBySign(outHitPoint.distance, distance, mask);
            outHitPoint.triangleId = Vector8::SelectBySign(outHitPoint.triangleId, Vector8::Splat(triangleIndex), mask);
            outHitPoint.objectId = Vector8::SelectBySign(outHitPoint.objectId, vInstanceID, mask);
        }
    }
}

void Mesh::Traverse_Single(const Ray& ray, HitPoint& data) const
{
    float distanceA, distanceB;

    if (mBVH.GetNumNodes() == 0)
    {
        // tree is empty
        return;
    }

    // all nodes
    const BVH::Node* nodes = mBVH.GetNodes();

    // "nodes to visit" stack
    Uint32 stackSize = 0;
    const BVH::Node* nodesStack[BVH::MaxDepth];

    // BVH traversal
    for (const BVH::Node* currentNode = nodes;;)
    {
        if (currentNode->IsLeaf())
        {
            Traverse_Leaf_Single(ray, *currentNode, data);
        }
        else
        {
            const BVH::Node* childA = nodes + currentNode->data.childIndex;
            const BVH::Node* childB = childA + 1;

            // prefetch grand-children
            RT_PREFETCH(nodes + childA->data.childIndex);
            RT_PREFETCH(nodes + childB->data.childIndex);

            bool hitA = Intersect_BoxRay(ray, childA->GetBox(), distanceA);
            bool hitB = Intersect_BoxRay(ray, childB->GetBox(), distanceB);

            // box occlusion
            hitA &= (distanceA < data.distance);
            hitB &= (distanceB < data.distance);

            if (hitA && hitB)
            {
                // will push [childA, childB] or [childB, childA] depending on distances
                const bool order = distanceA < distanceB;
                nodesStack[stackSize++] = order ? childB : childA; // push node
                currentNode = order ? childA : childB;
                continue;
            }
            else if (hitA)
            {
                currentNode = childA;
                continue;
            }
            else if (hitB)
            {
                currentNode = childB;
                continue;
            }
        }

        if (stackSize == 0)
        {
            break;
        }

        // pop a node
        currentNode = nodesStack[--stackSize];
    }
}

void Mesh::Traverse_Simd8(const math::Ray_Simd8& ray, HitPoint_Simd8& hitPoint, Uint32 instanceID) const
{
    // all nodes
    const BVH::Node* nodes = mBVH.GetNodes();

    // "nodes to visit" stack
    Uint32 stackSize = 0;
    const BVH::Node* nodesStack[BVH::MaxDepth];

    // BVH traversal
    for (const BVH::Node* currentNode = nodes;;)
    {
        if (currentNode->IsLeaf())
        {
            Traverse_Leaf_Simd8(ray, *currentNode, instanceID, hitPoint);
        }
        else
        {
            const BVH::Node* childA = nodes + currentNode->data.childIndex;
            const BVH::Node* childB = childA + 1;

            // prefetch grand-children
            RT_PREFETCH(nodes + childA->data.childIndex);
            RT_PREFETCH(nodes + childB->data.childIndex);

            // 4 rays - 2 boxes test
            Vector8 distanceA, distanceB;
            const Vector8 maskA = Intersect_BoxRay_Simd8(ray, childA->GetBox_Simd8(), hitPoint.distance, distanceA);
            const Vector8 maskB = Intersect_BoxRay_Simd8(ray, childB->GetBox_Simd8(), hitPoint.distance, distanceB);

            const int intMaskA = maskA.GetSignMask();
            const int intMaskB = maskB.GetSignMask();
            const int intMaskAB = intMaskA & intMaskB;

            if (intMaskAB)
            {
                const Vector8 orderMask(_mm256_cmp_ps(distanceA, distanceB, _CMP_LT_OQ));
                const int intOrderMask = orderMask.GetSignMask();
                const int orderMaskA = intOrderMask & intMaskAB;
                const int orderMaskB = (~intOrderMask) & intMaskAB;

                // traverse to child node A if majority rays hit it before the child B
                if (__popcnt(orderMaskA) > __popcnt(orderMaskB))
                {
                    nodesStack[stackSize++] = childB;
                    currentNode = childA;
                }
                else
                {
                    nodesStack[stackSize++] = childA;
                    currentNode = childB;
                }

                continue;
            }
            else if (intMaskA)
            {
                currentNode = childA;
                continue;
            }
            else if (intMaskB)
            {
                currentNode = childB;
                continue;
            }
        }

        if (stackSize == 0)
        {
            break;
        }

        // pop a node
        currentNode = nodesStack[--stackSize];
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void Mesh::Traverse_Packet(const RayPacket& packet, HitPoint_Packet& data, Uint32 instanceID) const
{
    (void)packet;
    (void)data;
    (void)instanceID;

    /*
    // all nodes
    const BVH::Node* nodes = mBVH.GetNodes();

    struct StackItem
    {
        const BVH::Node* node;
        Uint32 numActiveRays;
        Uint16 activeRays[MaxRayPacketSize];
    };

    // "nodes to visit" stack
    Uint32 stackSize = 0;
    StackItem nodesStack[BVH::MaxDepth];

    // BVH traversal
    for (const BVH::Node* currentNode = nodes;;)
    {
        if (currentNode->IsLeaf())
        {
            float distance, u, v;

            for (Uint32 i = 0; i < currentNode->data.numLeaves; ++i)
            {
                const Uint32 triangleIndex = node.data.childIndex + i;

                VertexIndices indices;
                mVertexBuffer.GetVertexIndices(triangleIndex, indices);

                Triangle tri;
                mVertexBuffer.GetVertexPositions(indices, tri);

                if (Intersect_TriangleRay(ray, tri, u, v, distance))
                {
                    if (distance < data.distance)
                    {
                        data.distance = distance;
                        data.triangle = triangleIndex;
                        data.u = u;
                        data.v = v;
                    }
                }
            }
        }
        else
        {
            const BVH::Node* childA = nodes + currentNode->data.childIndex;
            const BVH::Node* childB = childA + 1;

            // prefetch grand-children
            RT_PREFETCH(nodes + childA->data.childIndex);
            RT_PREFETCH(nodes + childB->data.childIndex);

            for (Uint32 i = 0; i < numActiveRays; ++i)
            {
                const Ray& ray = packet.rays[activeRays[i]];
            }

            bool hitA = Intersect_BoxRay(ray, childA->GetBox(), distanceA);
            bool hitB = Intersect_BoxRay(ray, childB->GetBox(), distanceB);

            // box occlusion
            hitA &= (distanceA < data.distance);
            hitB &= (distanceB < data.distance);

            if (hitA && hitB)
            {
                // will push [childA, childB] or [childB, childA] depending on distances
                const bool order = distanceA < distanceB;
                nodesStack[stackSize++] = order ? childB : childA; // push node
                currentNode = order ? childA : childB;
                continue;
            }
            else if (hitA)
            {
                currentNode = childA;
                continue;
            }
            else if (hitB)
            {
                currentNode = childB;
                continue;
            }
        }

        if (stackSize == 0)
        {
            break;
        }

        // pop a node
        currentNode = nodesStack[--stackSize];
    }
    */
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void Mesh::EvaluateShadingData_Single(const HitPoint& hitPoint, ShadingData& outShadingData) const
{
    const Material* material = mVertexBuffer.GetMaterial(hitPoint.triangleId);
    outShadingData.material = material ? material : &gDefaultMaterial;

    VertexIndices indices;
    mVertexBuffer.GetVertexIndices(hitPoint.triangleId, indices); // TODO cache this in MeshIntersectionData?

    Triangle normals, tangents, texCoords;
    mVertexBuffer.GetVertexTexCoords(indices, texCoords);
    mVertexBuffer.GetVertexNormals(indices, normals);
    mVertexBuffer.GetVertexTangents(indices, tangents);

    const Vector4 coeff0 = Vector4::Splat(1.0f - hitPoint.u - hitPoint.v);
    const Vector4 coeff1 = Vector4::Splat(hitPoint.u);
    const Vector4 coeff2 = Vector4::Splat(hitPoint.v);

    outShadingData.texCoord = coeff0 * texCoords.v0 + coeff1 * texCoords.v1 + coeff2 * texCoords.v2;
    outShadingData.normal = coeff0 * normals.v0 + coeff1 * normals.v1 + coeff2 * normals.v2;
    outShadingData.tangent = coeff0 * tangents.v0 + coeff1 * tangents.v1 + coeff2 * tangents.v2;
    outShadingData.bitangent = Vector4::Cross3(outShadingData.tangent, outShadingData.normal);

    outShadingData.normal.FastNormalize3();
    outShadingData.tangent.FastNormalize3();
    outShadingData.bitangent.FastNormalize3();
}

rt::math::Vector4 ShadingData::LocalToWorld(const math::Vector4 localCoords) const
{
    return tangent * localCoords[0] + bitangent * localCoords[1] + normal * localCoords[2];
}

math::Vector4 ShadingData::WorldToLocal(const math::Vector4 worldCoords) const
{
    // TODO optimize transposition
    const Vector4 worldToLocalX = Vector4(tangent[0], bitangent[0], normal[0]);
    const Vector4 worldToLocalY = Vector4(tangent[1], bitangent[1], normal[1]);
    const Vector4 worldToLocalZ = Vector4(tangent[2], bitangent[2], normal[2]);

    return worldToLocalX * worldCoords[0] + worldToLocalY * worldCoords[1] + worldToLocalZ * worldCoords[2];
}

} // namespace rt
