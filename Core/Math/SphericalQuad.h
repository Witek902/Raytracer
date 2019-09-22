#pragma once

#include "../RayLib.h"
#include "Vector4.h"

namespace rt {
namespace math {

// A helper structure for sampling a point on 3D quad
// that has uniform solid-angle distribution.
// Based on paper "An Area-Preserving Parametrization for Spherical Rectangles" by
// Carlos Urena, Marcos Fajardo and Alan King
struct SphericalQuad
{
    Vector4 o, x, y, z; // local reference system ’R’
    float z0; //
    float x0, y0; // rectangle coords in ’R’
    float x1, y1; //
    float b0, b1, k; // misc precomputed constants
    float S; // solid angle of ’Q’

    RT_INLINE void Init(const Vector4& s, const Vector4& ex, const Vector4& ey, const Vector4& o);
    RT_INLINE const Vector4 Sample(float u, float v, float& outPdf) const;
};

void SphericalQuad::Init(const Vector4& s, const Vector4& ex, const Vector4& ey, const Vector4& ref)
{
    o = ref;
    float exl = ex.Length3(), eyl = ey.Length3();
    // compute local reference system ’R’
    x = ex / exl;
    y = ey / eyl;
    z = Vector4::Cross3(x, y);
    // compute rectangle coords in local reference system
    Vector4 d = s - ref;
    z0 = Vector4::Dot3(d, z);
    // flip ’z’ to make it point against ’Q’
    if (z0 > 0.0f)
    {
        z *= -1.0f;
        z0 *= -1.0f;
    }
    x0 = Vector4::Dot3(d, x);
    y0 = Vector4::Dot3(d, y);
    x1 = x0 + exl;
    y1 = y0 + eyl;
    // create vectors to four vertices
    const Vector4 v00 = Vector4(x0, y0, z0, 0.0f);
    const Vector4 v01 = Vector4(x0, y1, z0, 0.0f);
    const Vector4 v10 = Vector4(x1, y0, z0, 0.0f);
    const Vector4 v11 = Vector4(x1, y1, z0, 0.0f);
    // compute normals to edges
    const Vector4 n0 = Vector4::Cross3(v00, v10).Normalized3();
    const Vector4 n1 = Vector4::Cross3(v10, v11).Normalized3();
    const Vector4 n2 = Vector4::Cross3(v11, v01).Normalized3();
    const Vector4 n3 = Vector4::Cross3(v01, v00).Normalized3();
    // compute internal angles (gamma_i)
    float g0 = acosf(-Vector4::Dot3(n0, n1));
    float g1 = acosf(-Vector4::Dot3(n1, n2));
    float g2 = acosf(-Vector4::Dot3(n2, n3));
    float g3 = acosf(-Vector4::Dot3(n3, n0));
    // compute predefined constants
    b0 = n0.z;
    b1 = n2.z;
    k = RT_2PI - g2 - g3;
    // compute solid angle from internal angles
    S = g0 + g1 - k;
}

RT_FORCE_NOINLINE
const Vector4 SphericalQuad::Sample(float u, float v, float& outPdf) const
{
    outPdf = 1.0f / Max(RT_EPSILON, S);

    // 1. compute ’cu’
    float au = u * S + k;
    float fu = (cosf(au) * b0 - b1) / sinf(au);
    float cu = 1.0f / sqrtf(fu * fu + b0 * b0) * (fu > 0.0f ? 1.0f : -1.0f);
    cu = Clamp(cu, -1.0f, 1.0f); // avoid NaNs
    // 2. compute ’xu’
    float xu = -(cu * z0) / sqrtf(1.0f - cu * cu);
    xu = Clamp(xu, x0, x1); // avoid Infs
    // 3. compute ’yv’
    float d = sqrtf(xu * xu + z0 * z0);
    float h0 = y0 / sqrtf(d * d + y0 * y0);
    float h1 = y1 / sqrtf(d * d + y1 * y1);
    float hv = h0 + v * (h1 - h0);
    float hv2 = hv * hv;
    float yv = (hv2 < 1.0f - RT_EPSILON) ? (hv * d) / sqrtf(1.0f - hv2) : y1;
    // 4. transform (xu,yv,z0) to world coords
    return o + xu * x + yv * y + z0 * z;
}

} // namespace math
} // namespace rt
