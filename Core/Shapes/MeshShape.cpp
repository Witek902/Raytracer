#include "PCH.h"

#include "MeshShape.h"
#include "BVH/BVHBuilder.h"

#include "Rendering/Context.h"
#include "Rendering/ShadingData.h"
#include "Traversal/TraversalContext.h"
#include "Traversal/Traversal_Single.h"

#include "Math/Geometry.h"
#include "Math/Simd8Geometry.h"

#include "Utils/Logger.h"


namespace rt {

using namespace math;

MeshShape::MeshShape()
{
}

MeshShape::~MeshShape()
{
}

const Box MeshShape::GetBoundingBox() const
{
    return mBoundingBox;
}

bool MeshShape::Initialize(const MeshDesc& desc)
{
    mBoundingBox = Box::Empty();

    const Float3* positions = desc.vertexBufferDesc.positions;
    const Uint32* indexBuffer = desc.vertexBufferDesc.vertexIndexBuffer;

    DynArray<Box> boxes;
    boxes.Reserve(desc.vertexBufferDesc.numTriangles);
    for (Uint32 i = 0; i < desc.vertexBufferDesc.numTriangles; ++i)
    {
        const Vector4 v0(positions[indexBuffer[3 * i + 0]]);
        const Vector4 v1(positions[indexBuffer[3 * i + 1]]);
        const Vector4 v2(positions[indexBuffer[3 * i + 2]]);

        Box triBox(v0, v1, v2);

        boxes.PushBack(triBox);

        mBoundingBox = Box(mBoundingBox, triBox);
    }

    BVHBuilder::BuildingParams params;
    params.maxLeafNodeSize = 2;

    BVHBuilder::Indices newTrianglesOrder;
    BVHBuilder bvhBuilder(mBVH);
    if (!bvhBuilder.Build(boxes.Data(), desc.vertexBufferDesc.numTriangles, params, newTrianglesOrder))
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
        for (Uint32 i = 0; i < stats.leavesCountHistogram.Size(); ++i)
        {
            if (i > 0)
                str << ", ";
            str << i << " (" << stats.leavesCountHistogram[i] << ")";
        }
        RT_LOG_INFO("    - leaf nodes histogram: %s", str.str().c_str());
    }

    // reorder triangles
    {
        DynArray<Uint32> newIndexBuffer(desc.vertexBufferDesc.numTriangles * 3);
        DynArray<Uint32> newMaterialIndexBuffer(desc.vertexBufferDesc.numTriangles);
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
        vertexBufferDesc.vertexIndexBuffer = newIndexBuffer.Data();
        vertexBufferDesc.materialIndexBuffer = newMaterialIndexBuffer.Data();

        if (!mVertexBuffer.Initialize(vertexBufferDesc))
        {
            RT_LOG_ERROR("Failed to initialize vertex buffer");
            return false;
        }
    }

    // TODO reorder indices

    RT_LOG_INFO("MeshShape '%s' created successfully", !desc.path.empty() ? desc.path.c_str() : "unnamed");
    return true;
}

float MeshShape::GetSurfaceArea() const
{
    RT_FATAL("Not implemented yet");
    return 0.0f;
}

const Vector4 MeshShape::Sample(const Float3& u, math::Vector4* outNormal, float* outPdf) const
{
    RT_FATAL("Not implemented yet");
    RT_UNUSED(u);
    RT_UNUSED(outPdf);
    RT_UNUSED(outNormal);
    return Vector4::Zero();
}

void MeshShape::Traverse(const SingleTraversalContext& context, const Uint32 objectID) const
{
    GenericTraverse<MeshShape>(context, objectID, this);
}

void MeshShape::Traverse_Leaf(const SingleTraversalContext& context, const Uint32 objectID, const BVH::Node& node) const
{
    float distance, u, v;

#ifdef RT_ENABLE_INTERSECTION_COUNTERS
    context.context.localCounters.numRayTriangleTests += node.numLeaves;
#endif // RT_ENABLE_INTERSECTION_COUNTERS

    const Uint32 numLeaves = node.numLeaves;
    const Uint32 childIndex = node.childIndex;

    for (Uint32 i = 0; i < numLeaves; ++i)
    {
        const Uint32 triangleIndex = childIndex + i;
        const ProcessedTriangle& tri = mVertexBuffer.GetTriangle(triangleIndex);

        if (Intersect_TriangleRay(context.ray, Vector4(&tri.v0.x), Vector4(&tri.edge1.x), Vector4(&tri.edge2.x), u, v, distance))
        {
            HitPoint& hitPoint = context.hitPoint;

            if (distance < hitPoint.distance)
            {
                hitPoint.distance = distance;
                hitPoint.subObjectId = triangleIndex;
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

bool MeshShape::Traverse_Shadow(const SingleTraversalContext& context) const
{
    return GenericTraverse_Shadow<MeshShape>(context, this);
}

bool MeshShape::Traverse_Leaf_Shadow(const SingleTraversalContext& context, const BVH::Node& node) const
{
    float distance, u, v;

#ifdef RT_ENABLE_INTERSECTION_COUNTERS
    context.context.localCounters.numRayTriangleTests += node.numLeaves;
#endif // RT_ENABLE_INTERSECTION_COUNTERS

    const Uint32 numLeaves = node.numLeaves;
    const Uint32 childIndex = node.childIndex;

    for (Uint32 i = 0; i < numLeaves; ++i)
    {
        const Uint32 triangleIndex = childIndex + i;
        const ProcessedTriangle& tri = mVertexBuffer.GetTriangle(triangleIndex);
        if (Intersect_TriangleRay(context.ray, Vector4(&tri.v0.x), Vector4(&tri.edge1.x), Vector4(&tri.edge2.x), u, v, distance))
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

/*
void MeshShape::Traverse_Leaf_Simd8(const SimdTraversalContext& context, const Uint32 objectID, const BVH::Node& node) const
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
            hitPoint.subObjectId = VectorInt8::SelectBySign(hitPoint.subObjectId, triangleIndexVec, VectorInt8::Cast(mask));
            hitPoint.objectId = VectorInt8::SelectBySign(hitPoint.objectId, objectIndexVec, VectorInt8::Cast(mask));

#ifdef RT_ENABLE_INTERSECTION_COUNTERS
            context.context.localCounters.numPassedRayTriangleTests += PopCount(intMask);
#endif // RT_ENABLE_INTERSECTION_COUNTERS
        }
    }
}
*/

void MeshShape::Traverse_Leaf(const PacketTraversalContext& context, const Uint32 objectID, const BVH::Node& node, const Uint32 numActiveGroups) const
{
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

            const VectorBool8 mask = Intersect_TriangleRay_Simd8(rayGroup.rays[1].dir, rayGroup.rays[1].origin, tri, rayGroup.maxDistances, u, v, distance);

            context.StoreIntersection(rayGroup, distance, u, v, mask, objectID, triangleIndex);

#ifdef RT_ENABLE_INTERSECTION_COUNTERS
            context.context.localCounters.numPassedRayTriangleTests += PopCount(mask.GetMask());
#endif // RT_ENABLE_INTERSECTION_COUNTERS
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MeshShape::EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outData) const
 {
    VertexIndices indices;
    mVertexBuffer.GetVertexIndices(hitPoint.subObjectId, indices);

    if (indices.materialIndex != UINT32_MAX)
    {
        outData.material = mVertexBuffer.GetMaterial(indices.materialIndex);
    }

    VertexShadingData vertexShadingData[3];
    mVertexBuffer.GetShadingData(indices, vertexShadingData[0], vertexShadingData[1], vertexShadingData[2]);

    const Vector4 coeff1 = Vector4(hitPoint.u);
    const Vector4 coeff2 = Vector4(hitPoint.v);
    const Vector4 coeff0 = Vector4(VECTOR_ONE) - (coeff1 + coeff2);

    const Vector4 texCoord0(vertexShadingData[0].texCoord);
    const Vector4 texCoord1(vertexShadingData[1].texCoord);
    const Vector4 texCoord2(vertexShadingData[2].texCoord);
    Vector4 texCoord = coeff1 * texCoord1;
    texCoord = Vector4::MulAndAdd(coeff2, texCoord2, texCoord);
    texCoord = Vector4::MulAndAdd(coeff0, texCoord0, texCoord);
    RT_ASSERT(texCoord.IsValid());
    outData.texCoord = texCoord;

    const Vector4 tangent0(vertexShadingData[0].tangent);
    const Vector4 tangent1(vertexShadingData[1].tangent);
    const Vector4 tangent2(vertexShadingData[2].tangent);
    Vector4 tangent = coeff1 * tangent1;
    tangent = Vector4::MulAndAdd(coeff2, tangent2, tangent);
    tangent = Vector4::MulAndAdd(coeff0, tangent0, tangent);
    tangent.FastNormalize3();
    RT_ASSERT(tangent.IsValid());
    outData.frame[0] = tangent;

    const Vector4 normal0(vertexShadingData[0].normal);
    const Vector4 normal1(vertexShadingData[1].normal);
    const Vector4 normal2(vertexShadingData[2].normal);
    Vector4 normal = coeff1 * normal1;
    normal = Vector4::MulAndAdd(coeff2, normal2, normal);
    normal = Vector4::MulAndAdd(coeff0, normal0, normal);
    normal.Normalize3();
    RT_ASSERT(normal.IsValid());
    outData.frame[2] = normal;
}

} // namespace rt
