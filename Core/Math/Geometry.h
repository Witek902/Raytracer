#pragma once

#include "../RayLib.h"
#include "Math.h"
#include "Vector4.h"
#include "Ray.h"
#include "Box.h"
#include "Triangle.h"


namespace rt {
namespace math {

RT_FORCE_INLINE constexpr float UniformHemispherePdf()
{
    return RT_INV_PI / 2.0f;
}

RT_FORCE_INLINE constexpr float UniformSpherePdf()
{
    return RT_INV_PI / 4.0f;
}

RT_FORCE_INLINE constexpr float UniformSpherePdf(const float radius)
{
    return RT_INV_PI / (4.0f * radius * radius);
}

RT_FORCE_INLINE constexpr float UniformCirclePdf(const float radius)
{
    return 1.0f / (RT_PI * Sqr(radius));
}

RT_FORCE_INLINE constexpr float SphereCapPdf(const float cosTheta)
{
    return 1.0f / (RT_2PI * (1.0f - cosTheta));
}

// Given a normalized vector 'n', generate orthonormal vectors 'u' and 'v'
RT_INLINE void BuildOrthonormalBasis(const Vector4& n, Vector4& u, Vector4& v)
{
    // algorithm based on "Building an Orthonormal Basis, Revisited" (2017) paper
    // by T. Duff, J. Burgess, P. Christensen, C. Hery, A. Kensler, M. Liani, and R. Villemin

    const float sign = CopySign(1.0f, n.z);
    const float a = -1.0f / (sign + n.z);

    u = Vector4(
        1.0f + sign * n.x * n.x * a,
        sign * n.x * n.y * a,
        -sign * n.x);

    v = Vector4(
        n.x * n.y * a,
        sign + n.y * n.y * a,
        -n.y);
}

RT_INLINE float PointLineDistanceSqr(const Vector4& pointOnLine, const Vector4& lineDir, const Vector4& testPoint)
{
    const Vector4 t = testPoint - pointOnLine;
    return Vector4::Cross3(lineDir, t).SqrLength3() / lineDir.SqrLength3();
}

RT_FORCE_INLINE bool Intersect_BoxRay(const Ray& ray, const Box& box, float& outDistance)
{
    // The algorithm is based on "slabs" method. More info can be found here:
    // http://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter3.htm

    // calculate all box planes distances
    Vector4 tmp1 = Vector4::MulAndSub(box.min, ray.invDir, ray.originDivDir); // box.min * ray.invDir - ray.originDivDir;
    Vector4 tmp2 = Vector4::MulAndSub(box.max, ray.invDir, ray.originDivDir); // box.max * ray.invDir - ray.originDivDir;
    Vector4 lmin = Vector4::Min(tmp1, tmp2);
    Vector4 lmax = Vector4::Max(tmp1, tmp2);

    // transpose (we need to calculate min and max of X, Y and Z)
    Vector4 lx = _mm_shuffle_ps(lmin, lmax, _MM_SHUFFLE(0, 0, 0, 0));
    Vector4 ly = _mm_shuffle_ps(lmin, lmax, _MM_SHUFFLE(1, 1, 1, 1));
    Vector4 lz = _mm_shuffle_ps(lmin, lmax, _MM_SHUFFLE(2, 2, 2, 2));

    // calculate minimum and maximum plane distances by taking min and max
    // of all 3 components
    lmin = Vector4::Max(lx, Vector4::Max(ly, lz));
    lmax = Vector4::Min(lx, Vector4::Min(ly, lz));
    outDistance = lmin.x;

    // setup data for final comparison
    lmax = lmax.SplatZ();
    // lmin = [lmin, 0, lmin, 0]
    lmin = _mm_unpacklo_ps(lmin, _mm_setzero_ps());

    // lmax >= lmin && lmax >= 0.0
    // (bits 0, 2)     (bit 1, 3)
    // The check below is a little bit redundant (we perform 4 comparisons), so
    // all 4 comparisons must return success (that's why we check if mask is 0xF).
    return (lmax >= lmin).All();
}

RT_FORCE_INLINE bool Intersect_BoxRay_TwoSided(const Ray& ray, const Box& box, float& outNearDist, float& outFarDist)
{
    // The algorithm is based on "slabs" method. More info can be found here:
    // http://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter3.htm

    // calculate all box planes distances
    Vector4 tmp1 = Vector4::MulAndSub(box.min, ray.invDir, ray.originDivDir); // box.min * ray.invDir - ray.originDivDir;
    Vector4 tmp2 = Vector4::MulAndSub(box.max, ray.invDir, ray.originDivDir); // box.max * ray.invDir - ray.originDivDir;
    Vector4 lmin = Vector4::Min(tmp1, tmp2);
    Vector4 lmax = Vector4::Max(tmp1, tmp2);

    // transpose (we need to calculate min and max of X, Y and Z)
    Vector4 lx = _mm_shuffle_ps(lmin, lmax, _MM_SHUFFLE(0, 0, 0, 0));
    Vector4 ly = _mm_shuffle_ps(lmin, lmax, _MM_SHUFFLE(1, 1, 1, 1));
    Vector4 lz = _mm_shuffle_ps(lmin, lmax, _MM_SHUFFLE(2, 2, 2, 2));

    // calculate minimum and maximum plane distances by taking min and max
    // of all 3 components
    lmin = Vector4::Max(lx, Vector4::Max(ly, lz));
    lmax = Vector4::Min(lx, Vector4::Min(ly, lz));

    outNearDist = lmin.x;
    outFarDist  = lmax.z;

    return outNearDist < outFarDist;
}

RT_FORCE_INLINE bool Intersect_TriangleRay(
    const Ray& ray,
    const Vector4& vertex0, const Vector4& edge01, const Vector4& edge02,
    float& outU, float& outV, float& outDistance)
{
    // Based on "Fast, Minimum Storage Ray/Triangle Intersection" by Tomas Möller and Ben Trumbore.

    // calculate distance from vert0 to ray origin
    Vector4 tvec = ray.origin - vertex0;

    // begin calculating determinant - also used to calculate U parameter
    Vector4 pvec = Vector4::Cross3(ray.dir, edge02);
    // if determinant is near zero, ray lies in plane of triangle
    Vector4 det = Vector4::Dot3V(edge01, pvec);
    // calculate U parameter
    Vector4 u = Vector4::Dot3V(tvec, pvec);


    // prepare to test V parameter
    Vector4 qvec = Vector4::Cross3(tvec, edge01);
    // calculate V parameter
    Vector4 v = Vector4::Dot3V(ray.dir, qvec);
    // calculate t (distance)
    Vector4 t = Vector4::Dot3V(edge02, qvec);

    // prepare data to the final comparison
    Vector4 tmp1 = _mm_unpacklo_ps(v, u); // [v, u, v, u]
    Vector4 tmp2 = _mm_unpacklo_ps(u + v, t); // [u+v, t, u+v, t]
    tmp1 = _mm_unpacklo_ps(tmp2, tmp1); // [u+v, v, t, u]
    tmp1 = tmp1 / det; // TODO this is slow, but reciprocal approximation gives bad results (artifacts)
    tmp2 = _mm_set_ss(1.0f); // [1.0, 0.0, 0.0, 0.0]

    outU = tmp1.w;
    outV = tmp1.y;
    outDistance = tmp1.z;

    // At this point, in tmp1 we have: [u, v, t, u + v]
    // and in tmp2 we have:            [0, 0, 0, 1].
    // The intersection occurs if (u > 0 && v > 0 && t > 0 && u + v <= 1),
    // so when performing SSE comparison 3 upper components must return true,
    // and last false, which yields to 0xE bit mask.
    return (tmp1 > tmp2).GetMask() == 0xE;
}


} // namespace math
} // namespace rt
