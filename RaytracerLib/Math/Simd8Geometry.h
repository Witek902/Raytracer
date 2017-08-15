#pragma once

#include "Simd8Ray.h"
#include "Simd8Box.h"
#include "Simd8Triangle.h"

namespace rt {
namespace math {

class RT_ALIGN(32) SimdRayBoxSegment8
{
public:
    Vector8 nearDists;
    Vector8 farDists;
};

RT_INLINE Vector8 Intersect_BoxRay_Simd8(const Ray_Simd8& ray, const Box_Simd8& box,
                                         SimdRayBoxSegment8& segment)
{
    // calculate all box planes distances
    const Vector3_Simd8 tmp1 = (box.min - ray.origin) * ray.invDir;
    const Vector3_Simd8 tmp2 = (box.max - ray.origin) * ray.invDir;
    const Vector3_Simd8 lmin = Vector3_Simd8::Min(tmp1, tmp2);
    const Vector3_Simd8 lmax = Vector3_Simd8::Max(tmp1, tmp2);

    // calculate minimum and maximum plane distances by taking min and max
    // of all 3 components
    const Vector8 maxDist = Vector8::Min(lmax.z, Vector8::Min(lmax.x, lmax.y));
    const Vector8 minDist = Vector8::Max(lmin.z, Vector8::Max(lmin.x, lmin.y));

    segment.nearDists = minDist;
    segment.farDists = maxDist;

    return Vector8(_mm256_cmp_ps(maxDist, _mm256_setzero_ps(), _CMP_GE_OQ))
        & Vector8(_mm256_cmp_ps(maxDist, minDist, _CMP_GE_OQ));
}

RT_INLINE Vector8 Intersect_TriangleRay_Simd8(const Ray_Simd8& ray, const Triangle_Simd8& tri,
                                              Vector8& outU, Vector8& outV, Vector8& outDist)
{
    // find vectors for two edges sharing v0
    const Vector3_Simd8 edge0 = tri.v1 - tri.v0;
    const Vector3_Simd8 edge1 = tri.v2 - tri.v0;

    // begin calculating determinant - also used to calculate U parameter
    const Vector3_Simd8 pvec = Vector3_Simd8::Cross(ray.dir, edge1);

    // if determinant is near zero, ray lies in plane of triangle
    const Vector8 det = Vector3_Simd8::Dot(edge0, pvec);
    const Vector8 invDet = Vector8(VECTOR8_ONE) / det;

    // calculate distance from vert0 to ray origin
    const Vector3_Simd8 tvec = ray.origin - tri.v0;

    // prepare to test V parameter
    const Vector3_Simd8 qvec = Vector3_Simd8::Cross(tvec, edge0);

    const Vector8 u = invDet * Vector3_Simd8::Dot(tvec, pvec);
    const Vector8 v = invDet * Vector3_Simd8::Dot(ray.dir, qvec);
    const Vector8 t = invDet * Vector3_Simd8::Dot(edge1, qvec);

    outU = u;
    outV = v;
    outDist = t;

    const Vector8 condA = _mm256_cmp_ps(u, _mm256_setzero_ps(), _CMP_GE_OQ);
    const Vector8 condB = _mm256_cmp_ps(v, _mm256_setzero_ps(), _CMP_GE_OQ);
    const Vector8 condC = _mm256_cmp_ps(u + v, VECTOR8_ONE, _CMP_LE_OQ);
    const Vector8 condD = _mm256_cmp_ps(t, _mm256_setzero_ps(), _CMP_GE_OQ);
    return (condA & condB) & (condC & condD);
}

} // namespace math
} // namespace rt
