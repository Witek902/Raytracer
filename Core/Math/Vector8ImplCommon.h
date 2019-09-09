#pragma once

namespace rt {
namespace math {

bool Vector8::AlmostEqual(const Vector8& v1, const Vector8& v2, float epsilon)
{
    return (Abs(v1 - v2) < Vector8(epsilon)).All();
}

const Vector8 Vector8::MulAndAdd(const Vector8& a, const float b, const Vector8& c)
{
    return MulAndAdd(a, Vector8(b), c);
}

const Vector8 Vector8::MulAndSub(const Vector8& a, const float b, const Vector8& c)
{
    return MulAndSub(a, Vector8(b), c);
}

const Vector8 Vector8::NegMulAndAdd(const Vector8& a, const float b, const Vector8& c)
{
    return NegMulAndAdd(a, Vector8(b), c);
}

const Vector8 Vector8::NegMulAndSub(const Vector8& a, const float b, const Vector8& c)
{
    return NegMulAndSub(a, Vector8(b), c);
}

const Vector8 Vector8::Lerp(const Vector8& v1, const Vector8& v2, const Vector8& weight)
{
    return MulAndAdd(v2 - v1, weight, v1);
}

const Vector8 Vector8::Lerp(const Vector8& v1, const Vector8& v2, float weight)
{
    return MulAndAdd(v2 - v1, weight, v1);
}

const Vector8 Vector8::Clamped(const Vector8& min, const Vector8& max) const
{
    return Min(max, Max(min, *this));
}

bool Vector8::IsValid() const
{
    return !IsNaN() && !IsInfinite();
}

} // namespace math
} // namespace rt
