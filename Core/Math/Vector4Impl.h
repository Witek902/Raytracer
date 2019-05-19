#pragma once

#include "Vector4.h"

namespace rt {
namespace math {

Float2 Vector4::ToFloat2() const
{
    return Float2{ x, y };
}

Float3 Vector4::ToFloat3() const
{
    return Float3{ x, y, z };
}

const Vector4 Vector4::SplatX() const
{
    return Swizzle<0, 0, 0, 0>();
}

const Vector4 Vector4::SplatY() const
{
    return Swizzle<1, 1, 1, 1>();
}

const Vector4 Vector4::SplatZ() const
{
    return Swizzle<2, 2, 2, 2>();
}

const Vector4 Vector4::SplatW() const
{
    return Swizzle<3, 3, 3, 3>();
}

const Vector4 Vector4::MulAndAdd(const Vector4& a, const float b, const Vector4& c)
{
    return MulAndAdd(a, Vector4(b), c);
}

const Vector4 Vector4::MulAndSub(const Vector4& a, const float b, const Vector4& c)
{
    return MulAndSub(a, Vector4(b), c);
}

const Vector4 Vector4::NegMulAndAdd(const Vector4& a, const float b, const Vector4& c)
{
    return NegMulAndAdd(a, Vector4(b), c);
}

const Vector4 Vector4::NegMulAndSub(const Vector4& a, const float b, const Vector4& c)
{
    return NegMulAndSub(a, Vector4(b), c);
}

const Vector4 Vector4::Lerp(const Vector4& v1, const Vector4& v2, const Vector4& weight)
{
    return MulAndAdd(v2 - v1, weight, v1);
}

const Vector4 Vector4::Lerp(const Vector4& v1, const Vector4& v2, float weight)
{
    return MulAndAdd(v2 - v1, Vector4(weight), v1);
}

const Vector4 Vector4::Saturate(const Vector4& v)
{
    return Min(Vector4(1.0f), Max(Vector4::Zero(), v));
}

const Vector4 Vector4::Clamp(const Vector4& x, const Vector4& min, const Vector4& max)
{
    return Min(max, Max(min, x));
}

float Vector4::SqrLength2() const
{
    return Dot2(*this, *this);
}

const Vector4 Vector4::Normalized3() const
{
    Vector4 result = *this;
    result.Normalize3();
    return result;
}

const Vector4 Vector4::InvNormalized(Vector4& outInvNormalized) const
{
    const Vector4 len = Length3V();
    const Vector4 temp = Vector4::Select<0, 0, 0, 1>(*this, len);
    const Vector4 invTemp = Vector4::Reciprocal(temp); // [1/x, 1/y, 1/y, 1/length]

    outInvNormalized = len * invTemp;
    return (*this) * invTemp.w;
}

const Vector4 Vector4::FastNormalized3() const
{
    Vector4 result = *this;
    result.FastNormalize3();
    return result;
}

float Vector4::SqrLength4() const
{
    return Dot4(*this, *this);
}

const Vector4 Vector4::Normalized4() const
{
    Vector4 result = *this;
    result.Normalize4();
    return result;
}

const Vector4 Vector4::Reflect3(const Vector4& i, const Vector4& n)
{
    // return (i - 2.0f * Dot(i, n) * n);
    const Vector4 vDot = Dot3V(i, n);
    return NegMulAndAdd(vDot + vDot, n, i);
}

bool Vector4::AlmostEqual(const Vector4& v1, const Vector4& v2, float epsilon)
{
    return (Abs(v1 - v2) < Vector4(epsilon)).All();
}

const VectorBool4 Vector4::IsZero() const
{
    return *this == Vector4::Zero();
}

} // namespace math
} // namespace rt
