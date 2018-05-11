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

RT_FORCE_INLINE Vector8 Intersect_BoxRay_Simd8(const Ray_Simd8& ray, const Box_Simd8& box, const Vector8& maxDistance, Vector8& outDistance)
{
    // same as:
    // const Vector3_Simd8 tmp1 = box.min * ray.invDir - ray.originDivDir;
    // const Vector3_Simd8 tmp2 = box.max * ray.invDir - ray.originDivDir;
    const Vector3_Simd8 tmp1 = Vector3_Simd8::MulAndSub(box.min, ray.invDir, ray.originDivDir);
    const Vector3_Simd8 tmp2 = Vector3_Simd8::MulAndSub(box.max, ray.invDir, ray.originDivDir);

    // TODO we can get rid of this 3 mins and 3 maxes by sorting the rays into octants
    // and processing each octant separately
    const Vector3_Simd8 lmax = Vector3_Simd8::Max(tmp1, tmp2);
    const Vector3_Simd8 lmin = Vector3_Simd8::Min(tmp1, tmp2);

    // calculate minimum and maximum plane distances by taking min and max of all 3 components
    const Vector8 maxT = Vector8::Min(lmax.z, Vector8::Min(lmax.x, lmax.y));
    const Vector8 minT = Vector8::Max(lmin.z, Vector8::Max(lmin.x, lmin.y));

    outDistance = minT;

    // return (maxT > 0 && minT < maxT && maxT < maxDistance)
    const Vector8 cond = _mm256_cmp_ps(Vector8::Min(maxDistance, maxT), minT, _CMP_GE_OQ);
    return _mm256_andnot_ps(maxT, cond); // trick: replace greater-than-zero compare with and-not
}

RT_INLINE Vector8 Intersect_TriangleRay_Simd8(const Ray_Simd8& ray, const Triangle_Simd8& tri, const Vector8& maxDistance,
                                              Vector8& outU, Vector8& outV, Vector8& outDist)
{
    const Vector8 one = VECTOR8_ONE;

    // begin calculating determinant - also used to calculate U parameter
    const Vector3_Simd8 pvec = Vector3_Simd8::Cross(ray.dir, tri.edge2);

    // if determinant is near zero, ray lies in plane of triangle
    const Vector8 det = Vector3_Simd8::Dot(tri.edge1, pvec);
    const Vector8 invDet = one / det;

    // calculate distance from vert0 to ray origin
    const Vector3_Simd8 tvec = ray.origin - tri.v0;

    // prepare to test V parameter
    const Vector3_Simd8 qvec = Vector3_Simd8::Cross(tvec, tri.edge1);

    const Vector8 u = invDet * Vector3_Simd8::Dot(tvec, pvec);
    const Vector8 v = invDet * Vector3_Simd8::Dot(ray.dir, qvec);
    const Vector8 t = invDet * Vector3_Simd8::Dot(tri.edge2, qvec);

    outU = u;
    outV = v;
    outDist = t;

    // u > 0 && v > 0 && t > 0 && u + v < 1 && t < maxDist
    const Vector8 condA = _mm256_andnot_ps(u, _mm256_cmp_ps(u + v, one, _CMP_LE_OQ));
    const Vector8 condB = _mm256_andnot_ps(t, _mm256_cmp_ps(t, maxDistance, _CMP_LE_OQ));
    return _mm256_andnot_ps(v, _mm256_and_ps(condA, condB));
}

} // namespace math
} // namespace rt
