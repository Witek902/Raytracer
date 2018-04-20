#include "PCH.h"

#include "Mesh.h"

#include "BVH/BVHBuilder.h"

#include "Traversal/IntersectionData.h"
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

        boxes.emplace_back(tri.v0, tri.v1, tri.v2);
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

void Mesh::RayTrace_Leaf_Single(const math::Ray& ray, const BVH::Node& node, RayIntersectionData& data, LocalCounters& counters) const
{
    float distance, u, v;

    counters.numRayTriangleTests += node.data.numLeaves;

    for (Uint32 i = 0; i < node.data.numLeaves; ++i)
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

                counters.numPassedRayTriangleTests++;
            }
        }
    }
}

void Mesh::RayTrace_Single(const Ray& ray, RayIntersectionData& data, LocalCounters& counters) const
{
    float distanceA, distanceB;

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
            RayTrace_Leaf_Single(ray, *currentNode, data, counters);
        }
        else
        {
            counters.numRayBoxTests += 2;

            const BVH::Node* childA = nodes + currentNode->data.childIndex;
            const BVH::Node* childB = childA + 1;

            // prefetch grand-children
            _mm_prefetch((const char*)(nodes + childA->data.childIndex), _MM_HINT_T0);
            _mm_prefetch((const char*)(nodes + childB->data.childIndex), _MM_HINT_T0);

            bool hitA = Intersect_BoxRay(ray, childA->GetBox(), distanceA);
            bool hitB = Intersect_BoxRay(ray, childB->GetBox(), distanceB);

            // box occlusion
            hitA &= (distanceA < data.distance);
            hitB &= (distanceB < data.distance);

            if (hitA && hitB)
            {
                counters.numPassedRayBoxTests += 2;

                // will push [childA, childB] or [childB, childA] depending on distances
                const bool order = distanceA < distanceB;
                nodesStack[stackSize++] = order ? childB : childA; // push node
                currentNode = order ? childA : childB;
                continue;
            }
            else if (hitA)
            {
                counters.numPassedRayBoxTests++;
                currentNode = childA;
                continue;
            }
            else if (hitB)
            {
                counters.numPassedRayBoxTests++;
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

void Mesh::RayTrace_Packet(const RayPacket& packet, RayPacketIntersectionData& data, Uint32 instanceID) const
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
            _mm_prefetch((const char*)(nodes + childA->data.childIndex), _MM_HINT_T0);
            _mm_prefetch((const char*)(nodes + childB->data.childIndex), _MM_HINT_T0);

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

void Mesh::EvaluateShadingData_Single(const Ray& ray, const RayIntersectionData& intersectionData, ShadingData& outShadingData) const
{
    const Material* material = mVertexBuffer.GetMaterial(intersectionData.triangle);
    outShadingData.material = material ? material : &gDefaultMaterial;
    outShadingData.position = ray.origin + ray.dir * intersectionData.distance;

    VertexIndices indices;
    mVertexBuffer.GetVertexIndices(intersectionData.triangle, indices); // TODO cache this in MeshIntersectionData?

    Triangle normals, tangents, texCoords;
    mVertexBuffer.GetVertexTexCoords(indices, texCoords);
    mVertexBuffer.GetVertexNormals(indices, normals);
    mVertexBuffer.GetVertexTangents(indices, tangents);

    const Vector4 coeff0 = Vector4::Splat(1.0f - intersectionData.u - intersectionData.v);
    const Vector4 coeff1 = Vector4::Splat(intersectionData.u);
    const Vector4 coeff2 = Vector4::Splat(intersectionData.v);

    outShadingData.texCoord = coeff0 * texCoords.v0 + coeff1 * texCoords.v1 + coeff2 * texCoords.v2;
    outShadingData.normal = coeff0 * normals.v0 + coeff1 * normals.v1 + coeff2 * normals.v2;
    outShadingData.tangent = coeff0 * tangents.v0 + coeff1 * tangents.v1 + coeff2 * tangents.v2;
    outShadingData.binormal = Vector4::Cross3(outShadingData.tangent, outShadingData.normal);

    outShadingData.normal.FastNormalize3();
    outShadingData.tangent.FastNormalize3();
    outShadingData.binormal.FastNormalize3();
}

} // namespace rt
