#include "PCH.h"

#include "Mesh.h"
#include "Material/Material.h"

#include "BVH/BVHBuilder.h"

#include "Rendering/ShadingData.h"
#include "Rendering/Context.h"
#include "Traversal/TraversalContext.h"

#include "Math/Triangle.h"
#include "Math/Geometry.h"
#include "Math/Simd8Triangle.h"
#include "Math/Simd8Geometry.h"

#include "Utils/Logger.h"
#include "Utils/Bitmap.h"
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
            RT_ASSERT(newTriangleIndex < desc.vertexBufferDesc.numTriangles);

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

void Mesh::Traverse_Leaf_Single(const SingleTraversalContext& context, const Uint32 objectID, const BVH::Node& node) const
{
    float distance, u, v;

#ifdef RT_ENABLE_INTERSECTION_COUNTERS
    context.context.localCounters.numRayTriangleTests += node.numLeaves;
#endif // RT_ENABLE_INTERSECTION_COUNTERS

    for (Uint32 i = 0; i < node.numLeaves; ++i)
    {
        const Uint32 triangleIndex = node.childIndex + i;

        const ProcessedTriangle tri = mVertexBuffer.GetTriangle(triangleIndex);

        if (Intersect_TriangleRay(context.ray, Vector4(tri.v0), Vector4(tri.edge1), Vector4(tri.edge2), u, v, distance))
        {
            HitPoint& hitPoint = context.hitPoint;
            const bool sameSurface = hitPoint.triangleId == triangleIndex && hitPoint.objectId == objectID;
            if (distance < hitPoint.distance && !sameSurface)
            {
                hitPoint.distance = distance;
                hitPoint.triangleId = triangleIndex;
                hitPoint.objectId = objectID;
                hitPoint.u = u;
                hitPoint.v = v;

#ifdef RT_ENABLE_INTERSECTION_COUNTERS
                context.context.localCounters.numPassedRayTriangleTests++;
#endif // RT_ENABLE_INTERSECTION_COUNTERS
            }
        }
    }
}

bool Mesh::Traverse_Leaf_Shadow_Single(const SingleTraversalContext& context, const BVH::Node& node) const
{
    float distance, u, v;

#ifdef RT_ENABLE_INTERSECTION_COUNTERS
    context.context.localCounters.numRayTriangleTests += node.numLeaves;
#endif // RT_ENABLE_INTERSECTION_COUNTERS

    for (Uint32 i = 0; i < node.numLeaves; ++i)
    {
        const Uint32 triangleIndex = node.childIndex + i;
        const ProcessedTriangle tri = mVertexBuffer.GetTriangle(triangleIndex);
        if (Intersect_TriangleRay(context.ray, Vector4(tri.v0), Vector4(tri.edge1), Vector4(tri.edge2), u, v, distance))
        {
            HitPoint& hitPoint = context.hitPoint;
            if (distance < hitPoint.distance)
            {
                hitPoint.distance = distance;

#ifdef RT_ENABLE_INTERSECTION_COUNTERS
                context.context.localCounters.numPassedRayTriangleTests++;
#endif // RT_ENABLE_INTERSECTION_COUNTERS

                return true;
            }
        }
    }

    return false;
}

void Mesh::Traverse_Leaf_Simd8(const SimdTraversalContext& context, const Uint32 objectID, const BVH::Node& node) const
{
    const VectorInt8 objectIndexVec(objectID);

    Vector8 distance, u, v;
    Triangle_Simd8 tri;

#ifdef RT_ENABLE_INTERSECTION_COUNTERS
    context.context.localCounters.numRayTriangleTests += 8 * node.numLeaves;
#endif // RT_ENABLE_INTERSECTION_COUNTERS

    for (Uint32 i = 0; i < node.numLeaves; ++i)
    {
        HitPoint_Simd8& hitPoint = context.hitPoint;
        const Uint32 triangleIndex = node.childIndex + i;
        const VectorInt8 triangleIndexVec(triangleIndex);

        mVertexBuffer.GetTriangle(triangleIndex, tri);

        const Vector8 mask = Intersect_TriangleRay_Simd8(context.ray.dir, context.ray.origin, tri, hitPoint.distance, u, v, distance);
        const Uint32 intMask = mask.GetSignMask();

        // TODO triangle & object filtering
        if (intMask)
        {
            // combine results according to mask
            hitPoint.u = Vector8::SelectBySign(hitPoint.u, u, mask);
            hitPoint.v = Vector8::SelectBySign(hitPoint.v, v, mask);
            hitPoint.distance = Vector8::SelectBySign(hitPoint.distance, distance, mask);
            hitPoint.triangleId = VectorInt8::SelectBySign(hitPoint.triangleId, triangleIndexVec, VectorInt8::Cast(mask));
            hitPoint.objectId = VectorInt8::SelectBySign(hitPoint.objectId, objectIndexVec, VectorInt8::Cast(mask));

#ifdef RT_ENABLE_INTERSECTION_COUNTERS
            context.context.localCounters.numPassedRayTriangleTests += PopCount(intMask);
#endif // RT_ENABLE_INTERSECTION_COUNTERS
        }
    }
}

void Mesh::Traverse_Leaf_Packet(const PacketTraversalContext& context, const Uint32 objectID, const BVH::Node& node, const Uint32 numActiveGroups) const
{
    RT_UNUSED(context);

    const VectorInt8 objectIndexVec(objectID);

    Vector8 distance, u, v;
    Triangle_Simd8 tri;

#ifdef RT_ENABLE_INTERSECTION_COUNTERS
    context.context.localCounters.numRayTriangleTests += 8 * node.numLeaves * numActiveGroups;
#endif // RT_ENABLE_INTERSECTION_COUNTERS

    for (Uint32 i = 0; i < node.numLeaves; ++i)
    {
        const Uint32 triangleIndex = node.childIndex + i;
        const Vector8 triangleIndexVec(triangleIndex);

        mVertexBuffer.GetTriangle(triangleIndex, tri);

        for (Uint32 j = 0; j < numActiveGroups; ++j)
        {
            RayGroup& rayGroup = context.ray.groups[context.context.activeGroupsIndices[j]];

            const Vector8 mask = Intersect_TriangleRay_Simd8(rayGroup.rays.dir, rayGroup.rays.origin, tri, rayGroup.maxDistances, u, v, distance);
            const Uint32 intMask = mask.GetSignMask();

            // TODO triangle & object filtering
            if (intMask)
            {
                rayGroup.maxDistances = Vector8::SelectBySign(rayGroup.maxDistances, distance, mask);
                // TODO write object & triangle ID
                //rayGroup.objectIndex = Vector8::SelectBySign(rayGroup.objectIndex, objectIndexVec, mask);
                //rayGroup.triangleIndex = Vector8::SelectBySign(rayGroup.triangleIndex, triangleIndexVec, mask);

                // write back the intersection result
                for (Uint32 k = 0; k < 8; ++k)
                {
                    // TODO this is not very optimal...
                    if ((intMask >> k) & 1)
                    {
                        const Uint32 rayOffset = rayGroup.rayOffsets[k];
                        const Uint32 subOffset = rayOffset % 8; // offset within hit point group
                        HitPoint_Simd8& hitPoint = context.hitPoint[rayOffset / 8];

                        hitPoint.u[subOffset] = u[k];
                        hitPoint.v[subOffset] = v[k];
                        hitPoint.distance[subOffset] = distance[k];
                        hitPoint.triangleId[subOffset] = triangleIndex;
                        hitPoint.objectId[subOffset] = objectID;
                    }
                }

#ifdef RT_ENABLE_INTERSECTION_COUNTERS
                context.context.localCounters.numPassedRayTriangleTests += PopCount(intMask);
#endif // RT_ENABLE_INTERSECTION_COUNTERS
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void Mesh::EvaluateShadingData_Single(const HitPoint& hitPoint, ShadingData& outShadingData) const
 {
    VertexIndices indices;
    mVertexBuffer.GetVertexIndices(hitPoint.triangleId, indices); // TODO cache this in MeshIntersectionData?

    outShadingData.material = indices.materialIndex != -1 ? mVertexBuffer.GetMaterial(indices.materialIndex) : &gDefaultMaterial;

    VertexShadingData vertexShadingData[3];
    mVertexBuffer.GetShadingData(indices, vertexShadingData[0], vertexShadingData[1], vertexShadingData[2]);

    const Vector4 coeff1 = Vector4(hitPoint.u);
    const Vector4 coeff2 = Vector4(hitPoint.v);
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

    if (outShadingData.material->normalMap)
    {
        Vector4 localNormal = outShadingData.material->GetNormalVector(outShadingData.texCoord);

        // TODO normal map strength
        //localNormal = Vector4::Lerp(VECTOR_Z, localNormal, 0.5f);

        // transform normal vector
        outShadingData.normal = outShadingData.tangent * localNormal.x + outShadingData.bitangent * localNormal.y + outShadingData.normal * localNormal.z;
        outShadingData.normal.FastNormalize3();

        // orthogonalize tangent vector
        outShadingData.tangent -= Vector4::Dot3V(outShadingData.tangent, outShadingData.normal) * outShadingData.normal;
        outShadingData.tangent.FastNormalize3();

        outShadingData.bitangent = Vector4::Cross3(outShadingData.tangent, outShadingData.normal);
    }
}

const Vector4 ShadingData::LocalToWorld(const Vector4 localCoords) const
{
    Vector4 result = tangent * localCoords.x;
    result = Vector4::MulAndAdd(bitangent, localCoords.y, result);
    result = Vector4::MulAndAdd(normal, localCoords.z, result);
    return result;
}

const Vector4 ShadingData::WorldToLocal(const Vector4 worldCoords) const
{
    Vector4 worldToLocalX = tangent;
    Vector4 worldToLocalY = bitangent;
    Vector4 worldToLocalZ = normal;
    Vector4::Transpose3(worldToLocalX, worldToLocalY, worldToLocalZ);

    return worldToLocalX * worldCoords.x + worldToLocalY * worldCoords.y + worldToLocalZ * worldCoords.z;
}

} // namespace rt
