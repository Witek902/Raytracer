#include "PCH.h"
#include "CpuMesh.h"
#include "Logger.h"
#include "BVHBuilder.h"
#include "AlignmentAllocator.h"

#include "Math/Triangle.h"
#include "Math/Geometry.h"
#include "Math/Simd4Geometry.h"
#include "Math/Simd8Geometry.h"

#include <vector>
#include <sstream>


namespace rt {

using namespace math;

const static Material gDefaultMaterial;

CpuMesh::CpuMesh()
{
}

CpuMesh::~CpuMesh()
{
}

bool CpuMesh::Initialize(const MeshDesc& desc)
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

bool CpuMesh::RayTrace_Single(const Ray& ray, float maxDistance, MeshIntersectionData& outData) const
{
    float distance, u, v;
    bool intersectionFound = false;

    Uint32 stackSize = 1;
    Uint32 nodesStack[BVH::MaxDepth];
    nodesStack[0] = 0;

    const BVH::Node* nodes = mBVH.GetNodes();

    // BVH traversal
    while (stackSize > 0)
    {
        const Uint32 currentNodeIndex = nodesStack[--stackSize];
        const BVH::Node& currentNode = nodes[currentNodeIndex];

        if (!Intersect(ray, currentNode.GetBox(), distance))
        {
            continue;
        }

        if (currentNode.data.numLeaves > 0) // leaf node
        {
            for (Uint32 i = 0; i < currentNode.data.numLeaves; ++i)
            {
                const Uint32 triangleIndex = currentNode.data.childIndex + i;

                VertexIndices indices;
                mVertexBuffer.GetVertexIndices(triangleIndex, indices);

                Triangle tri;
                mVertexBuffer.GetVertexPositions(indices, tri);

                if (Intersect_TriangleRay(ray, tri, u, v, distance))
                {
                    if (distance < maxDistance)
                    {
                        maxDistance = distance;
                        outData.distance = distance;
                        outData.triangle = triangleIndex;
                        outData.u = u;
                        outData.v = v;
                        intersectionFound = true;
                    }
                }
            }
        }
        else // non-leaf node
        {
            nodesStack[stackSize++] = currentNode.data.childIndex;
            nodesStack[stackSize++] = currentNode.data.childIndex + 1;
        }

        // TODO prefetching
        // TODO child reordering (depending on ray direction)
    }


    /*
    if (trianglesLeft >= 8)
    {
        const Ray_Simd8 simdRay(ray);
        const Vector8 infinity = Vector8::Splat(FLT_MAX);

        do
        {
            VertexIndices indices;
            Triangle tri0, tri1, tri2, tri3, tri4, tri5, tri6, tri7;

            // extract 4 triangles
            {
                mVertexBuffer.GetVertexIndices(i, indices);
                mVertexBuffer.GetVertexPositions(indices, tri0);
                mVertexBuffer.GetVertexIndices(i + 1, indices);
                mVertexBuffer.GetVertexPositions(indices, tri1);
                mVertexBuffer.GetVertexIndices(i + 2, indices);
                mVertexBuffer.GetVertexPositions(indices, tri2);
                mVertexBuffer.GetVertexIndices(i + 3, indices);
                mVertexBuffer.GetVertexPositions(indices, tri3);
                mVertexBuffer.GetVertexIndices(i + 4, indices);
                mVertexBuffer.GetVertexPositions(indices, tri4);
                mVertexBuffer.GetVertexIndices(i + 5, indices);
                mVertexBuffer.GetVertexPositions(indices, tri5);
                mVertexBuffer.GetVertexIndices(i + 6, indices);
                mVertexBuffer.GetVertexPositions(indices, tri6);
                mVertexBuffer.GetVertexIndices(i + 7, indices);
                mVertexBuffer.GetVertexPositions(indices, tri7);
            }

            Vector8 u, v, t;
            const Triangle_Simd8 simdTriangle(tri0, tri1, tri2, tri3, tri4, tri5, tri6, tri7);
            const Vector8 result = Intersect_TriangleRay_Simd8(simdRay, simdTriangle, u, v, t);
            const int mask = result.GetSignMask();

            if (mask != 0)
            {
                const Vector8 dist = Vector8::SelectBySign(infinity, t, result);
                const Vector8 vmin = dist.HorizontalMin();
                const __m256 vcmp = _mm256_cmp_ps(dist, vmin, _CMP_EQ_OQ);
                int compareMask = _mm256_movemask_ps(vcmp);

                unsigned long index = 0;
                _BitScanForward(&index, compareMask);

                if (vmin.f[0] < maxDistance)
                {
                    outData.distance = maxDistance = vmin.f[0];
                    outData.u = u[index];
                    outData.v = v[index];
                    outData.triangle = i + index;
                    intersectionFound = true;
                }
            }

            i += 8;
            trianglesLeft -= 8;
        } while (trianglesLeft >= 8);
    }

    if (trianglesLeft >= 4)
    {
        const Ray_Simd4 simdRay(ray);
        const Vector4 infinity = Vector4::Splat(FLT_MAX);

        do
        {
            VertexIndices indices;
            Triangle tri0, tri1, tri2, tri3;

            // extract 4 triangles
            {
                mVertexBuffer.GetVertexIndices(i, indices);
                mVertexBuffer.GetVertexPositions(indices, tri0);
                mVertexBuffer.GetVertexIndices(i + 1, indices);
                mVertexBuffer.GetVertexPositions(indices, tri1);
                mVertexBuffer.GetVertexIndices(i + 2, indices);
                mVertexBuffer.GetVertexPositions(indices, tri2);
                mVertexBuffer.GetVertexIndices(i + 3, indices);
                mVertexBuffer.GetVertexPositions(indices, tri3);
            }

            Vector4 u, v, t;
            const Triangle_Simd4 simdTriangle(tri0, tri1, tri2, tri3);
            const Vector4 result = Intersect_TriangleRay_Simd4(simdRay, simdTriangle, u, v, t);
            const int mask = result.GetSignMask();

            if (mask != 0)
            {
                const Vector4 dist = Vector4::SelectBySign(infinity, t, result);
                const Vector4 vmin = dist.HorizontalMin();
                const __m128 vcmp = _mm_cmpeq_ps(dist, vmin);
                int compareMask = _mm_movemask_ps(vcmp);

                unsigned long index = 0;
                _BitScanForward(&index, compareMask);
                if (vmin.f[0] < maxDistance)
                {
                    outData.distance = maxDistance = vmin.f[0];
                    outData.u = u[index];
                    outData.v = v[index];
                    outData.triangle = i + index;
                    intersectionFound = true;
                }
            }

            i += 4;
            trianglesLeft -= 4;
        } while (trianglesLeft >= 4);
    }

    const Uint32 numTriangles = mVertexBuffer.GetNumTriangles();
    for (Uint32 i = 0; i < mVertexBuffer.GetNumTriangles(); ++i)
    {
        VertexIndices indices;
        mVertexBuffer.GetVertexIndices(i, indices);

        Triangle tri;
        mVertexBuffer.GetVertexPositions(indices, tri);

        if (Intersect(ray, tri, distance))
        {
            if (distance < maxDistance)
            {
                maxDistance = distance;
                outData.distance = distance;
                outData.triangle = i;
                // TODO uv calculation
                intersectionFound = true;
            }
        }
    }
    */

    return intersectionFound;
}

Uint8 CpuMesh::RayTrace_Simd4(const math::Ray_Simd4& rays, const math::Vector4& maxDistances, MeshIntersectionData_Simd4& outData) const
{
    // TODO

    RT_UNUSED(rays);
    RT_UNUSED(maxDistances);
    RT_UNUSED(outData);

    return 0;
}

Uint8 CpuMesh::RayTrace_Simd8(const math::Ray_Simd8& rays, const math::Vector8& maxDistances, MeshIntersectionData_Simd8& outData) const
{
    // TODO

    RT_UNUSED(rays);
    RT_UNUSED(maxDistances);
    RT_UNUSED(outData);

    return 0;
}

void CpuMesh::EvaluateShadingData_Single(const Ray& ray, const MeshIntersectionData& intersectionData, ShadingData& outShadingData) const
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
