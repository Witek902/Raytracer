#pragma once

#include "Simd4Ray.h"
#include "Simd4Box.h"
#include "Simd4Triangle.h"

namespace rt {
namespace math {

class RT_ALIGN(16) SimdRayBoxSegment4
{
public:
    Vector4 nearDists;
    Vector4 farDists;
};

RT_INLINE Vector4 Intersect_BoxRay_Simd4(const Ray_Simd4& ray, const Box_Simd4& box,
                                         SimdRayBoxSegment4& segment)
{
    // calculate all box planes distances
    const Vector3_Simd4 tmp1 = (box.min - ray.origin) * ray.invDir;
    const Vector3_Simd4 tmp2 = (box.max - ray.origin) * ray.invDir;
    const Vector3_Simd4 lmin = Vector3_Simd4::Min(tmp1, tmp2);
    const Vector3_Simd4 lmax = Vector3_Simd4::Max(tmp1, tmp2);

    // calculate minimum and maximum plane distances by taking min and max
    // of all 3 components
    const Vector4 maxDist = Vector4::Min(lmax.z, Vector4::Min(lmax.x, lmax.y));
    const Vector4 minDist = Vector4::Max(lmin.z, Vector4::Max(lmin.x, lmin.y));

    segment.nearDists = minDist;
    segment.farDists = maxDist;

    return Vector4(_mm_cmpge_ps(maxDist, _mm_setzero_ps())) & Vector4(_mm_cmpge_ps(maxDist, minDist));
}

RT_INLINE Vector4 Intersect_TriangleRay_Simd4(const Ray_Simd4& ray, const Triangle_Simd4& tri,
                                              Vector4& outU, Vector4& outV, Vector4& outDist)
{
    // find vectors for two edges sharing v0
    const Vector3_Simd4 edge0 = tri.v1 - tri.v0;
    const Vector3_Simd4 edge1 = tri.v2 - tri.v0;

    // begin calculating determinant - also used to calculate U parameter
    const Vector3_Simd4 pvec = Vector3_Simd4::Cross(ray.dir, edge1);

    // if determinant is near zero, ray lies in plane of triangle
    const Vector4 det = Vector3_Simd4::Dot(edge0, pvec);
    const Vector4 invDet = Vector4(VECTOR_ONE) / det;

    // calculate distance from vert0 to ray origin
    const Vector3_Simd4 tvec = ray.origin - tri.v0;

    // prepare to test V parameter
    const Vector3_Simd4 qvec = Vector3_Simd4::Cross(tvec, edge0);

    const Vector4 u = invDet * Vector3_Simd4::Dot(tvec, pvec);
    const Vector4 v = invDet * Vector3_Simd4::Dot(ray.dir, qvec);
    const Vector4 t = invDet * Vector3_Simd4::Dot(edge1, qvec);

    outU = u;
    outV = v;
    outDist = t;

    const Vector4 condA = _mm_cmpge_ps(u, _mm_setzero_ps());
    const Vector4 condB = _mm_cmpge_ps(v, _mm_setzero_ps());
    const Vector4 condC = _mm_cmple_ps(u + v, VECTOR_ONE);
    const Vector4 condD = _mm_cmpge_ps(t, _mm_setzero_ps());
    return _mm_and_ps(_mm_and_ps(condA, condB), _mm_and_ps(condC, condD));
}

} // namespace math
} // namespace rt
