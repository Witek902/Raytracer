#pragma once

#include "Float2.h"

#include <cassert>


namespace rt {
namespace math {


static_assert(sizeof(Float2) == 2 * sizeof(float), "Invalid Float2 size");


constexpr Float2::Float2()
    : x(0.0f), y(0.0f)
{ }

constexpr Float2::Float2(const float* src)
    : x(src[0]), y(src[1])
{ }

constexpr Float2::Float2(float x, float y)
    : x(x), y(y)
{ }

float Float2::Get(Uint32 index) const
{
    assert(index < 2);
    return (&x)[index];
}

float& Float2::Get(Uint32 index)
{
    assert(index < 2);
    return (&x)[index];
}

//////////////////////////////////////////////////////////////////////////

template<Uint32 ix, Uint32 iy>
Float2 Float2::Swizzle() const
{
    static_assert(ix < 4, "Invalid X element index");
    static_assert(iy < 4, "Invalid Y element index");

    return Float2((&x)[ix], (&x)[iy]);
}

template<Uint32 ix, Uint32 iy>
constexpr Float2 Float2::Blend(const Float2& a, const Float2& b)
{
    return Float2(ix == 0 ? a.x : b.x, iy == 0 ? a.y : b.y);
}

template<bool x, bool y>
constexpr Float2 Float2::ChangeSign() const
{
    return Float2(x ? -f[0] : f[0], y ? -f[1] : f[1]);
}

Float2 Float2::SelectBySign(const Float2& a, const Float2& b, const Float2& sel)
{
    return Float2(sel.x > 0.0f ? a.x : b.x, sel.y > 0.0f ? a.y : b.y);
}

constexpr Float2 Float2::SplatX() const
{
    return Float2(x, x);
}

constexpr Float2 Float2::SplatY() const
{
    return Float2(y, y);
}

constexpr Float2 Float2::Splat(float f)
{
    return Float2(f, f);
}


//////////////////////////////////////////////////////////////////////////

constexpr Float2 Float2::operator- () const
{
    return Float2(-x, -y);
}

constexpr Float2 Float2::operator+ (const Float2& b) const
{
    return Float2(x + b.x, y + b.y);
}

constexpr Float2 Float2::operator- (const Float2& b) const
{
    return Float2(x - b.x, y - b.y);
}

constexpr Float2 Float2::operator* (const Float2& b) const
{
    return Float2(x * b.x, y * b.y);
}

Float2 Float2::operator/ (const Float2& b) const
{
    // TODO make it constexpr
    assert(math::Abs(b.x) > FLT_EPSILON);
    assert(math::Abs(b.y) > FLT_EPSILON);

    return Float2(x / b.x, y / b.y);
}

constexpr Float2 Float2::operator* (float b) const
{
    return Float2(x * b, y * b);
}

Float2 Float2::operator/ (float b) const
{
    // TODO make it constexpr
    assert(math::Abs(b) > FLT_EPSILON);

    return Float2(x / b, y / b);
}

Float2 operator*(float a, const Float2& b)
{
    return Float2(a * b.x, a * b.y);
}

//////////////////////////////////////////////////////////////////////////

Float2& Float2::operator+= (const Float2& b)
{
    x += b.x;
    y += b.y;
    return *this;
}

Float2& Float2::operator-= (const Float2& b)
{
    x -= b.x;
    y -= b.y;
    return *this;
}

Float2& Float2::operator*= (const Float2& b)
{
    x *= b.x;
    y *= b.y;
    return *this;
}

Float2& Float2::operator/= (const Float2& b)
{
    assert(math::Abs(b.x) > FLT_EPSILON);
    assert(math::Abs(b.y) > FLT_EPSILON);

    x /= b.x;
    y /= b.y;
    return *this;
}

Float2& Float2::operator*= (float b)
{
    x *= b;
    y *= b;
    return *this;
}

Float2& Float2::operator/= (float b)
{
    assert(math::Abs(b) > FLT_EPSILON);

    x /= b;
    y /= b;
    return *this;
}

//////////////////////////////////////////////////////////////////////////

Float2 Float2::Floor(const Float2& v)
{
    return Float2(floorf(v.x), floorf(v.y));
}

Float2 Float2::Sqrt(const Float2& v)
{
    assert(v.x >= 0.0f);
    assert(v.y >= 0.0f);

    return Float2(sqrtf(v.x), sqrtf(v.y));
}

Float2 Float2::Reciprocal(const Float2& v)
{
    assert(math::Abs(v.x) > FLT_EPSILON);
    assert(math::Abs(v.y) > FLT_EPSILON);

    // this checks are required to avoid "potential divide by 0" warning
    return Float2(v.x != 0.0f ? 1.0f / v.x : INFINITY,
                  v.y != 0.0f ? 1.0f / v.y : INFINITY);
}

constexpr Float2 Float2::Min(const Float2& a, const Float2& b)
{
    return Float2(math::Min<float>(a.x, b.x), math::Min<float>(a.y, b.y));
}

constexpr Float2 Float2::Max(const Float2& a, const Float2& b)
{
    return Float2(math::Max<float>(a.x, b.x), math::Max<float>(a.y, b.y));
}

constexpr Float2 Float2::Abs(const Float2& v)
{
    return Float2(math::Abs(v.x), math::Abs(v.y));
}

constexpr Float2 Float2::Lerp(const Float2& v1, const Float2& v2, const Float2& weight)
{
    return Float2(v1.x + weight.x * (v2.x - v1.x), v1.y + weight.y * (v2.y - v1.y));
}

constexpr Float2 Float2::Lerp(const Float2& v1, const Float2& v2, float weight)
{
    return Float2(v1.x + weight * (v2.x - v1.x), v1.y + weight * (v2.y - v1.y));
}

//////////////////////////////////////////////////////////////////////////

constexpr bool Float2::operator== (const Float2& b) const
{
    return (x == b.x) && (y == b.y);
}

constexpr bool Float2::operator< (const Float2& b) const
{
    return (x < b.x) && (y < b.y);
}

constexpr bool Float2::operator<= (const Float2& b) const
{
    return (x <= b.x) && (y <= b.y);
}

constexpr bool Float2::operator> (const Float2& b) const
{
    return (x > b.x) && (y > b.y);
}

constexpr bool Float2::operator>= (const Float2& b) const
{
    return (x >= b.x) && (y >= b.y);
}

constexpr bool Float2::operator!= (const Float2& b) const
{
    return (x != b.x) && (y != b.y);
}

constexpr bool Float2::AlmostEqual(const Float2& v1, const Float2& v2, float epsilon)
{
    return Abs(v1 - v2) < Float2::Splat(epsilon);
}

//////////////////////////////////////////////////////////////////////////

constexpr float Float2::Dot(const Float2& a, const Float2& b)
{
    return a.x * b.x + a.y * b.y;
}

constexpr float Float2::Cross(const Float2& a, const Float2& b)
{
    return a.x * b.y - a.y * b.x;
}

float Float2::Length() const
{
    return sqrtf(x * x + y * y);
}

Float2 Float2::Normalized() const
{
    const float len = Length();
    assert(len > FLT_EPSILON);

    const float lenInv = 1.0f / len;
    return *this * lenInv;
}

Float2& Float2::Normalize()
{
    const float len = Length();
    assert(len > FLT_EPSILON);

    const float lenInv = 1.0f / len;
    *this *= lenInv;
    return *this;
}

} // namespace math
} // namespace rt