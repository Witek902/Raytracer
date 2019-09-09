#pragma once

#include "Simd8Ray.h"
#include "Simd8Box.h"
#include "Simd8Triangle.h"

// Defining it will replace VBLENDVPS instruction with VMINPS/VMAXPS
// Makes the code faster on Haswell and Broadwell
// Probably not needed on Skylake (and higher) and AMD
#define RT_ARCH_SLOW_BLENDV

namespace rt {
namespace math {

template<Uint32 Octatnt>
RT_FORCE_INLINE const VectorBool8 Intersect_BoxRay_Simd8_Octant(
    const Vector3x8& rayInvDir,
    const Vector3x8& rayOriginDivDir,
    const Box_Simd8& box,
    const Vector8& maxDistance,
    Vector8& outDistance)
{
    static_assert(Octatnt < 8, "Invalid octant");

    const Vector3x8 tmp1 = Vector3x8::MulAndSub(box.min, rayInvDir, rayOriginDivDir);
    const Vector3x8 tmp2 = Vector3x8::MulAndSub(box.max, rayInvDir, rayOriginDivDir);

    constexpr Uint32 maskX = Octatnt & 1 ? 1 : 0;
    constexpr Uint32 maskY = Octatnt & 2 ? 1 : 0;
    constexpr Uint32 maskZ = Octatnt & 4 ? 1 : 0;

    Vector3x8 lmin, lmax;
    // lmax.x = _mm256_blend_ps(tmp2.x, tmp1.x, maskX);
    // lmax.y = _mm256_blend_ps(tmp2.y, tmp1.y, maskY);
    // lmax.z = _mm256_blend_ps(tmp2.z, tmp1.z, maskZ);
    // lmin.x = _mm256_blend_ps(tmp1.x, tmp2.x, maskX);
    // lmin.y = _mm256_blend_ps(tmp1.y, tmp2.y, maskY);
    // lmin.z = _mm256_blend_ps(tmp1.z, tmp2.z, maskZ);

    lmax.x = Vector8::Select<maskX,maskX,maskX,maskX>(tmp2.x, tmp1.x);
    lmax.y = Vector8::Select<maskY,maskY,maskY,maskY>(tmp2.y, tmp1.y);
    lmax.z = Vector8::Select<maskZ,maskZ,maskZ,maskZ>(tmp2.z, tmp1.z);
    lmin.x = Vector8::Select<maskX,maskX,maskX,maskX>(tmp1.x, tmp2.x);
    lmin.y = Vector8::Select<maskY,maskY,maskY,maskY>(tmp1.y, tmp2.y);
    lmin.z = Vector8::Select<maskZ,maskZ,maskZ,maskZ>(tmp1.z, tmp2.z);

    // calculate minimum and maximum plane distances by taking min and max of all 3 components
    const Vector8 maxT = Vector8::Min(lmax.z, Vector8::Min(lmax.x, lmax.y));
    const Vector8 minT = Vector8::Max(lmin.z, Vector8::Max(lmin.x, lmin.y));

    outDistance = minT;

#ifdef RT_USE_AVX
    const Vector8 cond = _mm256_cmp_ps(Vector8::Min(maxDistance, maxT), minT, _CMP_GE_OQ);
    return _mm256_andnot_ps(maxT, cond); // trick: replace greater-than-zero compare with and-not
#else
    const Vector8 zero = Vector8::Zero();
    return (maxT > zero) & (minT <= maxT) & (maxT <= maxDistance);
#endif
}

RT_FORCE_INLINE const VectorBool8 Intersect_BoxRay_Simd8(
    const Vector3x8& rayInvDir,
    const Vector3x8& rayOriginDivDir,
    const Box_Simd8& box,
    const Vector8& maxDistance,
    Vector8& outDistance)
{
    const Vector3x8 tmp1 = Vector3x8::MulAndSub(box.min, rayInvDir, rayOriginDivDir);
    const Vector3x8 tmp2 = Vector3x8::MulAndSub(box.max, rayInvDir, rayOriginDivDir);

    // TODO we can get rid of this 3 mins and 3 maxes by sorting the rays into octants
    // and processing each octant separately
#if defined(RT_ARCH_SLOW_BLENDV) || !defined(RT_USE_AVX)
    const Vector3x8 lmax = Vector3x8::Max(tmp1, tmp2);
    const Vector3x8 lmin = Vector3x8::Min(tmp1, tmp2);
#else // RT_ARCH_SLOW_BLENDV
    Vector3x8 lmin, lmax;
    lmax.x = _mm256_blendv_ps(tmp2.x, tmp1.x, rayInvDir.x);
    lmax.y = _mm256_blendv_ps(tmp2.y, tmp1.y, rayInvDir.y);
    lmax.z = _mm256_blendv_ps(tmp2.z, tmp1.z, rayInvDir.z);
    lmin.x = _mm256_blendv_ps(tmp1.x, tmp2.x, rayInvDir.x);
    lmin.y = _mm256_blendv_ps(tmp1.y, tmp2.y, rayInvDir.y);
    lmin.z = _mm256_blendv_ps(tmp1.z, tmp2.z, rayInvDir.z);
#endif // RT_ARCH_SLOW_BLENDV

    // calculate minimum and maximum plane distances by taking min and max of all 3 components
    const Vector8 maxT = Vector8::Min(lmax.z, Vector8::Min(lmax.x, lmax.y));
    const Vector8 minT = Vector8::Max(lmin.z, Vector8::Max(lmin.x, lmin.y));

    outDistance = minT;

#ifdef RT_USE_AVX
    // return (maxT > 0 && minT <= maxT && maxT <= maxDistance)
    const Vector8 cond = _mm256_cmp_ps(Vector8::Min(maxDistance, maxT), minT, _CMP_GE_OQ);
    return _mm256_andnot_ps(maxT, cond); // trick: replace greater-than-zero compare with and-not
#else
    const Vector8 zero = Vector8::Zero();
    return (maxT > zero) & (minT <= maxT) & (maxT <= maxDistance);
#endif
}

RT_FORCE_INLINE const VectorBool8 Intersect_BoxRay_TwoSided_Simd8(
    const Vector3x8& rayInvDir,
    const Vector3x8& rayOriginDivDir,
    const Box_Simd8& box,
    const Vector8& maxDistance,
    Vector8& outNearDist,
    Vector8& outFarDist)
{
    const Vector3x8 tmp1 = Vector3x8::MulAndSub(box.min, rayInvDir, rayOriginDivDir);
    const Vector3x8 tmp2 = Vector3x8::MulAndSub(box.max, rayInvDir, rayOriginDivDir);

#if defined(RT_ARCH_SLOW_BLENDV) || !defined(RT_USE_AVX)
    const Vector3x8 lmax = Vector3x8::Max(tmp1, tmp2);
    const Vector3x8 lmin = Vector3x8::Min(tmp1, tmp2);
#else // RT_ARCH_SLOW_BLENDV
    Vector3x8 lmin, lmax;
    lmax.x = _mm256_blendv_ps(tmp2.x, tmp1.x, rayInvDir.x);
    lmax.y = _mm256_blendv_ps(tmp2.y, tmp1.y, rayInvDir.y);
    lmax.z = _mm256_blendv_ps(tmp2.z, tmp1.z, rayInvDir.z);
    lmin.x = _mm256_blendv_ps(tmp1.x, tmp2.x, rayInvDir.x);
    lmin.y = _mm256_blendv_ps(tmp1.y, tmp2.y, rayInvDir.y);
    lmin.z = _mm256_blendv_ps(tmp1.z, tmp2.z, rayInvDir.z);
#endif // RT_ARCH_SLOW_BLENDV

    // calculate minimum and maximum plane distances by taking min and max of all 3 components
    const Vector8 maxT = Vector8::Min(lmax.z, Vector8::Min(lmax.x, lmax.y));
    const Vector8 minT = Vector8::Max(lmin.z, Vector8::Max(lmin.x, lmin.y));

    outNearDist = minT;
    outFarDist = maxT;

#ifdef RT_USE_AVX
    // return (maxT > 0 && minT <= maxT && maxT <= maxDistance)
    const Vector8 cond = _mm256_cmp_ps(Vector8::Min(maxDistance, maxT), minT, _CMP_GE_OQ);
    return VectorBool8(_mm256_andnot_ps(maxT, cond)); // trick: replace greater-than-zero compare with and-not
#else
    const Vector8 zero = Vector8::Zero();
    return (maxT > zero) & (minT <= maxT) & (maxT <= maxDistance);
#endif
}

RT_INLINE const VectorBool8 Intersect_TriangleRay_Simd8(
    const Vector3x8& rayDir,
    const Vector3x8& rayOrigin,
    const Triangle_Simd8& tri,
    const Vector8& maxDistance,
    Vector8& outU,
    Vector8& outV,
    Vector8& outDist)
{
    // M�ller�Trumbore algorithm

    const Vector8 one = VECTOR8_ONE;

    // begin calculating determinant - also used to calculate U parameter
    const Vector3x8 pvec = Vector3x8::Cross(rayDir, tri.edge2);

    // if determinant is near zero, ray lies in plane of triangle
    const Vector8 det = Vector3x8::Dot(tri.edge1, pvec);
    const Vector8 invDet = one / det;

    // calculate distance from vert0 to ray origin
    const Vector3x8 tvec = rayOrigin - tri.v0;

    // prepare to test V parameter
    const Vector3x8 qvec = Vector3x8::Cross(tvec, tri.edge1);

    const Vector8 u = invDet * Vector3x8::Dot(tvec, pvec);
    const Vector8 v = invDet * Vector3x8::Dot(rayDir, qvec);
    const Vector8 t = invDet * Vector3x8::Dot(tri.edge2, qvec);

    outU = u;
    outV = v;
    outDist = t;

#ifdef RT_USE_AVX
    // u > 0 && v > 0 && t > 0 && u + v < 1 && t < maxDist
    const Vector8 condA = _mm256_andnot_ps(u, _mm256_cmp_ps(t, maxDistance, _CMP_LT_OQ));
    const Vector8 condB = _mm256_andnot_ps(t, _mm256_cmp_ps(u + v, one, _CMP_LE_OQ));
    return _mm256_andnot_ps(v, _mm256_and_ps(condA, condB));
#else
    const Vector8 zero = Vector8::Zero();
    return (u > zero) & (v > zero) & (t > zero) & (u + v < one) & (t < maxDistance);
#endif
}


} // namespace math
} // namespace rt
