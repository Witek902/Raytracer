#include "PCH.h"
#include "CpuMesh.h"
#include "Logger.h"
#include "Math/Triangle.h"
#include "Math/Geometry.h"
#include "Math/Simd4Geometry.h"
#include "Math/Simd8Geometry.h"


namespace rt {

using namespace math;

const static Material gDefaultMaterial;

CpuMesh::CpuMesh()
    : mScale(1.0f)
    , mDebugName("mesh")
{ }

CpuMesh::~CpuMesh()
{
}

bool CpuMesh::Initialize(const MeshDesc& desc)
{
    if (!mVertexBuffer.Initialize(desc.vertexBufferDesc))
    {
        LOG_ERROR("Failed to initialize vertex buffer");
        return false;
    }

    mScale = Vector4::Splat(desc.scale);

    LOG_INFO("Mesh '%s' created successfully", desc.debugName ? desc.debugName : "unnamed");
    return true;
}

bool CpuMesh::RayTrace_Single(const Ray& ray, float maxDistance, MeshIntersectionData& outData) const
{
    bool intersectionFound = false;
    const Uint32 numTriangles = mVertexBuffer.GetNumTriangles();

    Uint32 i = 0;
    Uint32 trianglesLeft = numTriangles;

    /*

    */

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
    */

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


    for (; i < numTriangles; ++i)
    {
        VertexIndices indices;
        Triangle tri;
        mVertexBuffer.GetVertexIndices(i, indices);
        mVertexBuffer.GetVertexPositions(indices, tri);

        float distance;
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

    return intersectionFound;
}

void CpuMesh::EvaluateShadingData_Single(const Ray& ray, const MeshIntersectionData& intersectionData,
                                         ShadingData& outShadingData) const
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
    
    const Vector4 coeff0 = Vector4::Splat(intersectionData.u);
    const Vector4 coeff1 = Vector4::Splat(intersectionData.v);
    const Vector4 coeff2 = Vector4(VECTOR_ONE) - coeff0 - coeff1;

    // TODO use FMA
    outShadingData.texCoord = (coeff0 * texCoords.v0 + coeff1 * texCoords.v1 + coeff2 * texCoords.v2);
    outShadingData.normal = (coeff0 * normals.v0 + coeff1 * normals.v1 + coeff2 * normals.v2).FastNormalized3();
    outShadingData.tangent = (coeff0 * tangents.v0 + coeff1 * tangents.v1 + coeff2 * tangents.v2).FastNormalized3();
    outShadingData.binormal = Vector4::Cross3(outShadingData.tangent, outShadingData.normal).FastNormalized3();
}

} // namespace rt
