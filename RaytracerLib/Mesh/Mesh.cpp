#include "PCH.h"

#include "Mesh.h"
#include "Material/Material.h"

#include "BVH/BVHBuilder.h"

#include "Traversal/Traversal.h"
#include "Rendering/Counters.h"

#include "Math/Triangle.h"
#include "Math/Geometry.h"


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
    mBoundingBox = Box::Empty();

    const Float3* positions = (const Float3*)desc.vertexBufferDesc.positions;
    const Uint32* indexBuffer = desc.vertexBufferDesc.vertexIndexBuffer;

    std::vector<Box, AlignmentAllocator<Box>> boxes;
    for (Uint32 i = 0; i < desc.vertexBufferDesc.numTriangles; ++i)
    {
        const Vector4 v0(positions[indexBuffer[3 * i + 0]]);
        const Vector4 v1(positions[indexBuffer[3 * i + 1]]);
        const Vector4 v2(positions[indexBuffer[3 * i + 2]]);

        Box triBox(v0, v1, v2);
        const Vector4 size = triBox.max - triBox.min;
        triBox.min -= size * 0.001f;
        triBox.max += size * 0.001f;
        triBox.min -= VECTOR_EPSILON;
        triBox.max += VECTOR_EPSILON;

        boxes.push_back(triBox);

        mBoundingBox = Box(mBoundingBox, triBox);
    }

    BVHBuilder::BuildingParams params;
    params.maxLeafNodeSize = 2;

    BVHBuilder::Indices newTrianglesOrder;
    BVHBuilder bvhBuilder(mBVH);
    if (!bvhBuilder.Build(boxes.data(), desc.vertexBufferDesc.numTriangles, params, newTrianglesOrder))
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

    // reorder triangles
    {
        std::vector<Uint32> newIndexBuffer(desc.vertexBufferDesc.numTriangles * 3);
        std::vector<Uint32> newMaterialIndexBuffer(desc.vertexBufferDesc.numTriangles);
        for (Uint32 i = 0; i < desc.vertexBufferDesc.numTriangles; ++i)
        {
            const Uint32 newTriangleIndex = newTrianglesOrder[i];
            assert(newTriangleIndex < mNumTriangles);

            newIndexBuffer[3 * i] = indexBuffer[3 * newTriangleIndex];
            newIndexBuffer[3 * i + 1] = indexBuffer[3 * newTriangleIndex + 1];
            newIndexBuffer[3 * i + 2] = indexBuffer[3 * newTriangleIndex + 2];
            newMaterialIndexBuffer[i] = desc.vertexBufferDesc.materialIndexBuffer[newTriangleIndex];
        }

        VertexBufferDesc vertexBufferDesc = desc.vertexBufferDesc;
        vertexBufferDesc.vertexIndexBuffer = newIndexBuffer.data();
        vertexBufferDesc.materialIndexBuffer = newMaterialIndexBuffer.data();

        if (!mVertexBuffer.Initialize(vertexBufferDesc))
        {
            RT_LOG_ERROR("Failed to initialize vertex buffer");
            return false;
        }
    }

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

        const math::ProcessedTriangle tri = mVertexBuffer.GetTriangle(triangleIndex);

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

void Mesh::Traverse_Leaf_Simd8(const math::Ray_Simd8& ray, const BVH::Node& node, HitPoint_Simd8& outHitPoint) const
{
    Vector8 distance, u, v;
    Triangle_Simd8 tri;

    // TODO counters

    for (Uint32 i = 0; i < node.data.numLeaves; ++i)
    {
        const Uint32 triangleIndex = node.data.childIndex + i;

        mVertexBuffer.GetTriangle(triangleIndex, tri);

        const Vector8 mask = Intersect_TriangleRay_Simd8(ray, tri, outHitPoint.distance, u, v, distance);

        if (mask.GetSignMask())
        {
            // combine results according to mask
            outHitPoint.u = Vector8::SelectBySign(outHitPoint.u, u, mask);
            outHitPoint.v = Vector8::SelectBySign(outHitPoint.v, v, mask);
            outHitPoint.distance = Vector8::SelectBySign(outHitPoint.distance, distance, mask);
            outHitPoint.triangleId = Vector8::SelectBySign(outHitPoint.triangleId, Vector8::Splat(triangleIndex), mask);
        }
    }
}

void Mesh::Traverse_Single(const Ray& ray, HitPoint& outHitPoint) const
{
    GenericTraverse_Single(mBVH, ray, outHitPoint, this);
}

void Mesh::Traverse_Simd8(const math::Ray_Simd8& ray, HitPoint_Simd8& outHitPoint) const
{
    GenericTraverse_Simd8(mBVH, ray, outHitPoint, this);
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
    VertexIndices indices;
    mVertexBuffer.GetVertexIndices(hitPoint.triangleId, indices); // TODO cache this in MeshIntersectionData?

    const Material* material = mVertexBuffer.GetMaterial(indices.materialIndex);
    outShadingData.material = material ? material : &gDefaultMaterial;

    VertexShadingData vertexShadingData[3];
    mVertexBuffer.GetShadingData(indices, vertexShadingData[0], vertexShadingData[1], vertexShadingData[2]);

    const Vector4 coeff1 = Vector4::Splat(hitPoint.u);
    const Vector4 coeff2 = Vector4::Splat(hitPoint.v);
    const Vector4 coeff0 = Vector4(VECTOR_ONE) - coeff1 - coeff2;

    const Vector4 texCoord0(&vertexShadingData[0].texCoord.x);
    const Vector4 texCoord1(&vertexShadingData[1].texCoord.x);
    const Vector4 texCoord2(&vertexShadingData[2].texCoord.x);
    outShadingData.texCoord = coeff1 * texCoord1;
    outShadingData.texCoord = Vector4::MulAndAdd(coeff2, texCoord2, outShadingData.texCoord);
    outShadingData.texCoord = Vector4::MulAndAdd(coeff0, texCoord0, outShadingData.texCoord);

    const Vector4 normal0(&vertexShadingData[0].normal.x);
    const Vector4 normal1(&vertexShadingData[1].normal.x);
    const Vector4 normal2(&vertexShadingData[2].normal.x);
    outShadingData.normal = coeff1 * normal1;
    outShadingData.normal = Vector4::MulAndAdd(coeff2, normal2, outShadingData.normal);
    outShadingData.normal = Vector4::MulAndAdd(coeff0, normal0, outShadingData.normal);
    outShadingData.normal.FastNormalize3();

    const Vector4 tangent0(&vertexShadingData[0].tangent.x);
    const Vector4 tangent1(&vertexShadingData[1].tangent.x);
    const Vector4 tangent2(&vertexShadingData[2].tangent.x);
    outShadingData.tangent = coeff1 * tangent1;
    outShadingData.tangent = Vector4::MulAndAdd(coeff2, tangent2, outShadingData.tangent);
    outShadingData.tangent = Vector4::MulAndAdd(coeff0, tangent0, outShadingData.tangent);
    outShadingData.tangent.FastNormalize3();

    outShadingData.bitangent = Vector4::Cross3(outShadingData.tangent, outShadingData.normal);
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
