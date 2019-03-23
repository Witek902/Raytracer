#include "PCH.h"
#include "SamplingHelpers.h"
#include "Transcendental.h"
#include "VectorInt8.h"

namespace rt {
namespace math {

const Vector2x8 SamplingHelpers::GetCircle_Simd8(const Vector2x8& u)
{
    // angle (uniform distribution)
    const Vector8 theta = u.x * (2.0f * RT_PI);

    // radius (corrected distribution)
    const Vector8 r = Vector8::Sqrt(u.y);

    const Vector8 vSin = Sin(theta);
    const Vector8 vCos = Sin(theta + Vector8(RT_PI / 2.0f));

    return { r * vSin, r * vCos };
}

const Vector4 SamplingHelpers::GetTriangle(const Float2 u)
{
    const float t = sqrtf(u.x);
    return { 1.0f - t, u.y * t };
}

const Vector4 SamplingHelpers::GetCircle(const Float2 u)
{
    // angle (uniform distribution)
    const float theta = 2.0f * RT_PI * u.x;

    // radius (corrected distribution)
    const float r = sqrtf(u.y);

    return r * SinCos(theta);
}

const Vector4 SamplingHelpers::GetHexagon(const Float3 u)
{
    constexpr Float2 hexVectors[] =
    {
        { -1.0f, 0.0f },
        { 0.5f, 0.8660254f }, // sqrt(3.0f) / 2.0f
        { 0.5f, -0.8660254f }, // sqrt(3.0f) / 2.0f
        { -1.0f, 0.0f },
    };

    const Uint32 x = static_cast<Uint32>(3.0f * u.z);
    RT_ASSERT(x >= 0 && x < 3);

    const Float2 a = hexVectors[x];
    const Float2 b = hexVectors[x + 1];

    return Vector4(u.x * a.x + u.y * b.x, u.x * a.y + u.y * b.y, 0.0f, 0.0f);
}

const Vector2x8 SamplingHelpers::GetHexagon_Simd8(const Vector2x8& u1, const Vector8& u2)
{
    // TODO uint vector
    const VectorInt8 i = VectorInt8::Convert(3.0f * u2);
    const VectorInt8 j = i + 1;

    const Vector8 hexVectorsX(-1.0f, 0.5f, 0.5f, -1.0f, -1.0f, 0.5f, 0.5f, -1.0f);
    const Vector8 hexVectorsY(0.0f, 0.8660254f, -0.8660254f, 0.0f, 0.0f, 0.8660254f, -0.8660254f, 0.0f);
    const Vector2x8 x{ _mm256_permutevar_ps(hexVectorsX, i), _mm256_permutevar_ps(hexVectorsX, j) };
    const Vector2x8 y{ _mm256_permutevar_ps(hexVectorsY, i), _mm256_permutevar_ps(hexVectorsY, j) };

    return { Vector2x8::Dot(u1, x), Vector2x8::Dot(u1, y) };
}

/*
const Vector4 SamplingHelpers::GetRegularPolygon(const Uint32 n, const Vector4& u)
{
    RT_ASSERT(n >= 3, "Polygon must have at least 3 sides");

    // generate random point in a generic triangle
    const Float2 uv = GetVector4().ToFloat2();
    const float u = sqrtf(uv.x);
    const Float2 triangle(1.0f - u, uv.y * u);

    // base triangle size
    const float a = Sin(RT_PI / (float)n); // can be precomputed
    const float b = sqrtf(1.0f - a * a);

    // genrate point in base triangle
    const float sign = GetInt() % 2 ? 1.0f : -1.0f;
    const Vector4 base(b * (triangle.x + triangle.y), a * triangle.y * sign, 0.0f, 0.0f);

    // rotate
    const float alpha = RT_2PI * (float)(GetInt() % n) / (float)n;
    const Vector4 sinCosAlpha = SinCos(alpha);

    return Vector4(sinCosAlpha.y * base.x - sinCosAlpha.x * base.y, sinCosAlpha.y * base.y + sinCosAlpha.x * base.x, 0.0f, 0.0f);
}

const Vector2x8 SamplingHelpers::GetRegularPolygon_Simd8(const Uint32 n, const Vector2x8& u)
{
    RT_ASSERT(n >= 3, "Polygon must have at least 3 sides");

    const float invN = 1.0f / (float)n;

    // generate random point in a generic triangle
    const Vector8 t = Vector8::Sqrt(u.x);
    const Vector2x8 triangle(Vector8(1.0f) - t, u.y * t);

    // base triangle size
    const float a = Sin(RT_PI * invN); // can be precomputed
    const float b = sqrtf(1.0f - a * a);

    // genrate point in base triangle
    const float sign = GetInt() % 2 ? 1.0f : -1.0f;
    const Vector2x8 base(b * (triangle.x + triangle.y), a * triangle.y * sign);

    // rotate
    const VectorInt8 i = (u.w & VectorInt8(0x7FFFFFFF)) % n;
    const Vector8 alpha = i.ConvertToFloat() * (RT_2PI * invN);
    const Vector8 sinAlpha = Sin(alpha);
    const Vector8 cosAlpha = Cos(alpha);

    return Vector2x8(cosAlpha * base.x - sinAlpha * base.y, cosAlpha * base.y + sinAlpha * base.x);
}
*/

const Vector4 SamplingHelpers::GetSphere(const Float2 u)
{
    // based on http://mathworld.wolfram.com/SpherePointPicking.html

    // TODO 'u' should be already bipolar
    const Vector4 v = Vector4::MulAndSub(Vector4(u), 2.0f, VECTOR_ONE);

    const float t = sqrtf(1.0f - v.y * v.y);
    const float theta = RT_PI * v.x;
    Vector4 result = t * SinCos(theta); // xy
    result.z = v.y;

    return result;
}

const Vector4 SamplingHelpers::GetHemishpere(const Float2 u)
{
    Vector4 p = GetSphere(u);
    p.z = Abs(p.z);
    return p;
}

const Vector4 SamplingHelpers::GetHemishpereCos(const Float2 u)
{
    const float theta = 2.0f * RT_PI * u.y;
    const float r = sqrtf(u.x); // this is required for the result vector to be normalized

    Vector4 result = r * SinCos(theta); // xy
    result.z = sqrtf(1.0f - u.x);

    return result;
}

const Vector4 SamplingHelpers::GetFloatNormal2(const Float2 u)
{
    // Box-Muller method
    return sqrtf(-2.0f * FastLog(u.x)) * SinCos(2.0f * RT_PI * u.y);
}

} // namespace math
} // namespace rt
