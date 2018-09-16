#pragma once

#include "RayLib.h"
#include "Math.h"
#include "Vector4.h"
#include "Ray.h"
#include "Box.h"
#include "Triangle.h"


namespace rt {
namespace math {

RT_FORCE_INLINE bool Intersect_BoxRay(const Ray& ray, const Box& box, float& outDistance)
{
    // The algorithm is based on "slabs" method. More info can be found here:
    // http://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter3.htm

    Vector4 lmin, lmax, tmp1, tmp2;

    // calculate all box planes distances
    tmp1 = (box.min - ray.origin) * ray.invDir;
    tmp2 = (box.max - ray.origin) * ray.invDir;
    lmin = Vector4::Min(tmp1, tmp2);
    lmax = Vector4::Max(tmp1, tmp2);

    // transpose (we need to calculate min and max of X, Y and Z)
    Vector4 lx = _mm_shuffle_ps(lmin, lmax, _MM_SHUFFLE(0, 0, 0, 0));
    Vector4 ly = _mm_shuffle_ps(lmin, lmax, _MM_SHUFFLE(1, 1, 1, 1));
    Vector4 lz = _mm_shuffle_ps(lmin, lmax, _MM_SHUFFLE(2, 2, 2, 2));

    // calculate minimum and maximum plane distances by taking min and max
    // of all 3 components
    lmin = Vector4::Max(lx, Vector4::Max(ly, lz));
    lmax = Vector4::Min(lx, Vector4::Min(ly, lz));
    outDistance = lmin[0];

    // setup data for final comparison
    lmax = lmax.SplatZ();
    // lmin = [lmin, lmin, 0, 0]
    lmin = _mm_shuffle_ps(lmin, _mm_setzero_ps(), _MM_SHUFFLE(0, 0, 0, 0));

    // lmax >= lmin && lmax >= 0.0
    // (bits 0, 1)     (bit 2, 3)
    // The check below is a little bit redundant (we perform 4 comparisons), so
    // all 4 comparisons must return success (that's why we check if mask is 0xF).
    return _mm_movemask_ps(_mm_cmpge_ps(lmax, lmin)) == 0xF;
}

RT_FORCE_INLINE bool Intersect_BoxRay_TwoSided(const Ray& ray, const Box& box, float& outNearDist, float& outFarDist)
{
    // The algorithm is based on "slabs" method. More info can be found here:
    // http://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter3.htm

    Vector4 lmin, lmax, tmp1, tmp2;

    // calculate all box planes distances
    tmp1 = (box.min - ray.origin) * ray.invDir;
    tmp2 = (box.max - ray.origin) * ray.invDir;
    lmin = Vector4::Min(tmp1, tmp2);
    lmax = Vector4::Max(tmp1, tmp2);

    // transpose (we need to calculate min and max of X, Y and Z)
    Vector4 lx = _mm_shuffle_ps(lmin, lmax, _MM_SHUFFLE(0, 0, 0, 0));
    Vector4 ly = _mm_shuffle_ps(lmin, lmax, _MM_SHUFFLE(1, 1, 1, 1));
    Vector4 lz = _mm_shuffle_ps(lmin, lmax, _MM_SHUFFLE(2, 2, 2, 2));

    // calculate minimum and maximum plane distances by taking min and max
    // of all 3 components
    lmin = Vector4::Max(lx, Vector4::Max(ly, lz));
    lmax = Vector4::Min(lx, Vector4::Min(ly, lz));

    outNearDist = lmin[0];
    outFarDist  = lmax[2];

    return outNearDist < outFarDist;
}

RT_FORCE_INLINE bool Intersect_TriangleRay(const Ray& ray, const ProcessedTriangle& tri, float& outU, float& outV, float& outDistance)
{
    // Based on "Fast, Minimum Storage Ray/Triangle Intersection" by Tomas Möller and Ben Trumbore.

    // find vectors for two edges sharing v0
    const Vector4 edge0(tri.edge1);
    const Vector4 edge1(tri.edge2);

    // calculate distance from vert0 to ray origin
    Vector4 tvec = ray.origin - Vector4(tri.v0);

    // begin calculating determinant - also used to calculate U parameter
    Vector4 pvec = Vector4::Cross3(ray.dir, edge1);
    // if determinant is near zero, ray lies in plane of triangle
    Vector4 det = Vector4::Dot3V(edge0, pvec);
    // calculate U parameter
    Vector4 u = Vector4::Dot3V(tvec, pvec);

    // prepare to test V parameter
    Vector4 qvec = Vector4::Cross3(tvec, edge0);
    // calculate V parameter
    Vector4 v = Vector4::Dot3V(ray.dir, qvec);
    // calculate t (distance)
    Vector4 t = Vector4::Dot3V(edge1, qvec);

    // prepare data to the final comparison
    Vector4 tmp1 = _mm_shuffle_ps(v, u, _MM_SHUFFLE(0, 0, 0, 0));
    Vector4 tmp2 = _mm_shuffle_ps(u + v, t, _MM_SHUFFLE(0, 0, 0, 0));
    tmp1 = _mm_shuffle_ps(tmp2, tmp1, _MM_SHUFFLE(2, 0, 2, 0));
    tmp1 = tmp1 / det; // TODO this is slow, but reciprocal approximation gives bad results (artifacts)
    tmp2 = _mm_set_ss(1.0f);

    outU = tmp1[3];
    outV = tmp1[2];
    outDistance = tmp1[1];

    // At this point, in tmp1 we have: [u, v, t, u + v]
    // and in tmp2 we have:            [0, 0, 0, 1].
    // The intersection occurs if (u > 0 && v > 0 && t > 0 && u + v < 1),
    // so when performing SSE comparison 3 upper components must return true,
    // and last false, which yields to 0xE bit mask.
    return (_mm_movemask_ps(_mm_cmpgt_ps(tmp1, tmp2)) == 0xE);
}


} // namespace math
} // namespace rt
