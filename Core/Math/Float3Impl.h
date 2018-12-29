#pragma once

#include "Float3.h"

#include <cassert>


namespace rt {
namespace math {


static_assert(sizeof(Float3) == 3 * sizeof(float), "Invalid Float3 size");


constexpr Float3::Float3()
    : x(0.0f), y(0.0f), z(0.0f)
{ }

constexpr Float3::Float3(const float* src)
    : x(src[0]), y(src[1]), z(src[2])
{ }

constexpr Float3::Float3(const Float2& src)
    : x(src.x), y(src.y), z(0.0f)
{ }

constexpr Float3::Float3(float x, float y, float z)
    : x(x), y(y), z(z)
{ }

float Float3::Get(Uint32 index) const
{
    RT_ASSERT(index < 3);
    return (&x)[index];
}

float& Float3::Get(Uint32 index)
{
    RT_ASSERT(index < 3);
    return (&x)[index];
}

Float3::operator Float2() const
{
    return Float2(x, y);
}

bool Float3::IsValid() const
{
    return math::IsValid(x) && math::IsValid(y) && math::IsValid(z);
}

//////////////////////////////////////////////////////////////////////////

template<Uint32 ix, Uint32 iy, Uint32 iz>
const Float3 Float3::Swizzle() const
{
    static_assert(ix < 3, "Invalid X element index");
    static_assert(iy < 3, "Invalid Y element index");
    static_assert(iz < 3, "Invalid Z element index");

    return Float3((&x)[ix], (&x)[iy], (&x)[iz]);
}

template<Uint32 ix, Uint32 iy, Uint32 iz>
constexpr const Float3 Float3::Blend(const Float3& a, const Float3& b)
{
    return Float3(ix == 0 ? a.x : b.x,
                  iy == 0 ? a.y : b.y,
                  iz == 0 ? a.z : b.z);
}

template<bool x, bool y, bool z>
constexpr const Float3 Float3::ChangeSign() const
{
    return Float3(
        x ? -f[0] : f[0],
        y ? -f[1] : f[1],
        z ? -f[2] : f[2]
    );
}

const Float3 Float3::SelectBySign(const Float3& a, const Float3& b, const Float3& sel)
{
    Float3 ret;
    ret.x = sel.x > 0.0f ? a.x : b.x;
    ret.y = sel.y > 0.0f ? a.y : b.y;
    ret.z = sel.z > 0.0f ? a.z : b.z;
    return ret;
}

constexpr const Float3 Float3::SplatX() const
{
    return Float3(x, x, x);
}

constexpr const Float3 Float3::SplatY() const
{
    return Float3(y, y, y);
}

constexpr const Float3 Float3::SplatZ() const
{
    return Float3(z, z, z);
}

constexpr const Float3 Float3::Splat(float f)
{
    return Float3(f, f, f);
}


//////////////////////////////////////////////////////////////////////////

constexpr const Float3 Float3::operator- () const
{
    return Float3(-x, -y, -z);
}

constexpr const Float3 Float3::operator+ (const Float3& b) const
{
    return Float3(x + b.x, y + b.y, z + b.z);
}

constexpr const Float3 Float3::operator- (const Float3& b) const
{
    return Float3(x - b.x, y - b.y, z - b.z);
}

constexpr const Float3 Float3::operator* (const Float3& b) const
{
    return Float3(x * b.x, y * b.y, z * b.z);
}

const Float3 Float3::operator/ (const Float3& b) const
{
    // TODO make it constexpr
    RT_ASSERT(math::Abs(b.x) > FLT_EPSILON);
    RT_ASSERT(math::Abs(b.y) > FLT_EPSILON);
    RT_ASSERT(math::Abs(b.z) > FLT_EPSILON);

    return Float3(x / b.x, y / b.y, z / b.z);
}

constexpr const Float3 Float3::operator* (float b) const
{
    return Float3(x * b, y * b, z * b);
}

const Float3 Float3::operator/ (float b) const
{
    // TODO make it constexpr
    RT_ASSERT(math::Abs(b) > FLT_EPSILON);

    return Float3(x / b, y / b, z / b);
}

const Float3 operator*(float a, const Float3& b)
{
    return Float3(a * b.x, a * b.y, a * b.z);
}

//////////////////////////////////////////////////////////////////////////

Float3& Float3::operator+= (const Float3& b)
{
    x += b.x;
    y += b.y;
    z += b.z;
    return *this;
}

Float3& Float3::operator-= (const Float3& b)
{
    x -= b.x;
    y -= b.y;
    z -= b.z;
    return *this;
}

Float3& Float3::operator*= (const Float3& b)
{
    x *= b.x;
    y *= b.y;
    z *= b.z;
    return *this;
}

Float3& Float3::operator/= (const Float3& b)
{
    RT_ASSERT(math::Abs(b.x) > FLT_EPSILON);
    RT_ASSERT(math::Abs(b.y) > FLT_EPSILON);
    RT_ASSERT(math::Abs(b.z) > FLT_EPSILON);

    x /= b.x;
    y /= b.y;
    z /= b.z;
    return *this;
}

Float3& Float3::operator*= (float b)
{
    x *= b;
    y *= b;
    z *= b;
    return *this;
}

Float3& Float3::operator/= (float b)
{
    RT_ASSERT(math::Abs(b) > FLT_EPSILON);

    x /= b;
    y /= b;
    z /= b;
    return *this;
}

//////////////////////////////////////////////////////////////////////////

const Float3 Float3::Floor(const Float3& v)
{
    return Float3(floorf(v.x), floorf(v.y), floorf(v.z));
}

const Float3 Float3::Sqrt(const Float3& v)
{
    RT_ASSERT(v.x >= 0.0f);
    RT_ASSERT(v.y >= 0.0f);
    RT_ASSERT(v.z >= 0.0f);

    return Float3(sqrtf(v.x), sqrtf(v.y), sqrtf(v.z));
}

const Float3 Float3::Reciprocal(const Float3& v)
{
    RT_ASSERT(math::Abs(v.x) > FLT_EPSILON);
    RT_ASSERT(math::Abs(v.y) > FLT_EPSILON);
    RT_ASSERT(math::Abs(v.z) > FLT_EPSILON);

    // this checks are required to avoid "potential divide by 0" warning
    return Float3(v.x != 0.0f ? 1.0f / v.x : INFINITY,
                  v.y != 0.0f ? 1.0f / v.y : INFINITY,
                  v.z != 0.0f ? 1.0f / v.z : INFINITY);
}

constexpr const Float3 Float3::Min(const Float3& a, const Float3& b)
{
    return Float3(
        math::Min<float>(a.x, b.x),
        math::Min<float>(a.y, b.y),
        math::Min<float>(a.z, b.z)
    );
}

constexpr const Float3 Float3::Max(const Float3& a, const Float3& b)
{
    return Float3(
        math::Max<float>(a.x, b.x),
        math::Max<float>(a.y, b.y),
        math::Max<float>(a.z, b.z)
    );
}

constexpr const Float3 Float3::Abs(const Float3& v)
{
    return Float3(math::Abs(v.x), math::Abs(v.y), math::Abs(v.z));
}

constexpr const Float3 Float3::Lerp(const Float3& v1, const Float3& v2, const Float3& weight)
{
    return Float3(
        v1.x + weight.x * (v2.x - v1.x),
        v1.y + weight.y * (v2.y - v1.y),
        v1.z + weight.z * (v2.z - v1.z)
    );
}

constexpr const Float3 Float3::Lerp(const Float3& v1, const Float3& v2, float weight)
{
    return Float3(
        v1.x + weight * (v2.x - v1.x),
        v1.y + weight * (v2.y - v1.y),
        v1.z + weight * (v2.z - v1.z)
    );
}

//////////////////////////////////////////////////////////////////////////

constexpr bool Float3::operator== (const Float3& b) const
{
    return (x == b.x) && (y == b.y) && (z == b.z);
}

constexpr bool Float3::operator< (const Float3& b) const
{
    return (x < b.x) && (y < b.y) && (z < b.z);
}

constexpr bool Float3::operator<= (const Float3& b) const
{
    return (x <= b.x) && (y <= b.y) && (z <= b.z);
}

constexpr bool Float3::operator> (const Float3& b) const
{
    return (x > b.x) && (y > b.y) && (z > b.z);
}

constexpr bool Float3::operator>= (const Float3& b) const
{
    return (x >= b.x) && (y >= b.y) && (z >= b.z);
}

constexpr bool Float3::operator!= (const Float3& b) const
{
    return (x != b.x) && (y != b.y) && (z != b.z);
}

constexpr bool Float3::AlmostEqual(const Float3& v1, const Float3& v2, float epsilon)
{
    return Abs(v1 - v2) < Float3::Splat(epsilon);
}

//////////////////////////////////////////////////////////////////////////

constexpr float Float3::Dot(const Float3& a, const Float3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

constexpr const Float3 Float3::Cross(const Float3& a, const Float3& b)
{
    return Float3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

float Float3::Length() const
{
    return sqrtf(x * x + y * y + z * z);
}

const Float3 Float3::Normalized() const
{
    const float len = Length();
    RT_ASSERT(len > FLT_EPSILON);

    const float lenInv = 1.0f / len;
    return *this * lenInv;
}

Float3& Float3::Normalize()
{
    const float len = Length();
    RT_ASSERT(len > FLT_EPSILON);

    const float lenInv = 1.0f / len;
    *this *= lenInv;
    return *this;
}


} // namespace math
} // namespace rt