#pragma once

#include "Vector4.h"

namespace rt {
namespace math {

const VectorInt4 VectorInt4::Zero()
{
    return VectorInt4{ 0, 0, 0, 0 };
}

VectorInt4::VectorInt4(const VectorInt4& other)
    : x(other.x), y(other.y), z(other.z), w(other.w)
{}

VectorInt4::VectorInt4(const VectorBool4& other)
    : x(other.b[0] ? 0xFFFFFFFF : 0)
    , y(other.b[1] ? 0xFFFFFFFF : 0)
    , z(other.b[2] ? 0xFFFFFFFF : 0)
    , w(other.b[3] ? 0xFFFFFFFF : 0)
{}

const VectorInt4 VectorInt4::Cast(const Vector4& v)
{
    return reinterpret_cast<const VectorInt4&>(v);
}

const Vector4 VectorInt4::CastToFloat() const
{
    return reinterpret_cast<const Vector4&>(*this);
}

VectorInt4::VectorInt4(const Int32 x, const Int32 y, const Int32 z, const Int32 w)
    : i{ x, y, z, w }
{}

VectorInt4::VectorInt4(const Int32 i)
    : i{ i, i, i, i }
{}

VectorInt4::VectorInt4(const Uint32 u)
    : x(static_cast<Int32>(u))
    , y(static_cast<Int32>(u))
    , z(static_cast<Int32>(u))
    , w(static_cast<Int32>(u))
{}

const VectorInt4 VectorInt4::Convert(const Vector4& v)
{
    return
    {
        static_cast<Int32>(v.x),
        static_cast<Int32>(v.y),
        static_cast<Int32>(v.z),
        static_cast<Int32>(v.w)
    };
}

const VectorInt4 VectorInt4::TruncateAndConvert(const Vector4& v)
{
    // TODO
    return
    {
        static_cast<Int32>(v.x),
        static_cast<Int32>(v.y),
        static_cast<Int32>(v.z),
        static_cast<Int32>(v.w)
    };
}

const Vector4 VectorInt4::ConvertToFloat() const
{
    return Vector4
    {
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(z),
        static_cast<float>(w)
    };
}

//////////////////////////////////////////////////////////////////////////

const VectorInt4 VectorInt4::Select(const VectorInt4& a, const VectorInt4& b, const VectorBool4& sel)
{
    return
    {
        sel.Get<0>() ? b.x : a.x,
        sel.Get<1>() ? b.y : a.y,
        sel.Get<2>() ? b.z : a.z,
        sel.Get<3>() ? b.w : a.w,
    };
}

template<Uint32 ix, Uint32 iy, Uint32 iz, Uint32 iw>
const VectorInt4 VectorInt4::Swizzle() const
{
    static_assert(ix < 4, "Invalid X element index");
    static_assert(iy < 4, "Invalid Y element index");
    static_assert(iz < 4, "Invalid Z element index");
    static_assert(iw < 4, "Invalid W element index");

    return { i[ix], i[iy], i[iz], i[iw] };
}

//////////////////////////////////////////////////////////////////////////

const VectorInt4 VectorInt4::operator & (const VectorInt4& b) const
{
    return { x & b.x, y & b.y, z & b.z, w & b.w };
}

const VectorInt4 VectorInt4::AndNot(const VectorInt4& a, const VectorInt4& b)
{
    return { (~a.x) & b.x, (~a.y) & b.y, (~a.z) & b.z, (~a.w) & b.w };
}

const VectorInt4 VectorInt4::operator | (const VectorInt4& b) const
{
    return { x | b.x, y | b.y, z | b.z, w | b.w };
}

const VectorInt4 VectorInt4::operator ^ (const VectorInt4& b) const
{
    return { x ^ b.x, y ^ b.y, z ^ b.z, w ^ b.w };
}

VectorInt4& VectorInt4::operator &= (const VectorInt4& b)
{
    x &= b.x;
    y &= b.y;
    z &= b.z;
    w &= b.w;
    return *this;
}

VectorInt4& VectorInt4::operator |= (const VectorInt4& b)
{
    x |= b.x;
    y |= b.y;
    z |= b.z;
    w |= b.w;
    return *this;
}

VectorInt4& VectorInt4::operator ^= (const VectorInt4& b)
{
    x ^= b.x;
    y ^= b.y;
    z ^= b.z;
    w ^= b.w;
    return *this;
}

//////////////////////////////////////////////////////////////////////////

const VectorInt4 VectorInt4::operator - () const
{
    return VectorInt4::Zero() - (*this);
}

const VectorInt4 VectorInt4::operator + (const VectorInt4& b) const
{
    return { x + b.x, y + b.y, z + b.z, w + b.w };
}

const VectorInt4 VectorInt4::operator - (const VectorInt4& b) const
{
    return { x - b.x, y - b.y, z - b.z, w - b.w };
}

const VectorInt4 VectorInt4::operator * (const VectorInt4& b) const
{
    return { x * b.x, y * b.y, z * b.z, w * b.w };
}

VectorInt4& VectorInt4::operator += (const VectorInt4& b)
{
    x += b.x;
    y += b.y;
    z += b.z;
    w += b.w;
    return *this;
}

VectorInt4& VectorInt4::operator -= (const VectorInt4& b)
{
    x -= b.x;
    y -= b.y;
    z -= b.z;
    w -= b.w;
    return *this;
}

VectorInt4& VectorInt4::operator *= (const VectorInt4& b)
{
    x *= b.x;
    y *= b.y;
    z *= b.z;
    w *= b.w;
    return *this;
}

const VectorInt4 VectorInt4::operator + (Int32 b) const
{
    return { x + b, y + b, z + b, w + b };
}

const VectorInt4 VectorInt4::operator - (Int32 b) const
{
    return { x - b, y - b, z - b, w - b };
}

const VectorInt4 VectorInt4::operator * (Int32 b) const
{
    return { x * b, y * b, z * b, w * b };
}

VectorInt4& VectorInt4::operator += (Int32 b)
{
    x += b;
    y += b;
    z += b;
    w += b;
    return *this;
}

VectorInt4& VectorInt4::operator -= (Int32 b)
{
    x -= b;
    y -= b;
    z -= b;
    w -= b;
    return *this;
}

VectorInt4& VectorInt4::operator *= (Int32 b)
{
    x *= b;
    y *= b;
    z *= b;
    w *= b;
    return *this;
}

//////////////////////////////////////////////////////////////////////////

const VectorInt4 VectorInt4::operator << (const VectorInt4& b) const
{
    return { x << b.x, y << b.y, z << b.z, w << b.w };
}

const VectorInt4 VectorInt4::operator >> (const VectorInt4& b) const
{
    return { x >> b.x, y >> b.y, z >> b.z, w >> b.w };
}

VectorInt4& VectorInt4::operator <<= (const VectorInt4& b)
{
    x <<= b.x;
    y <<= b.y;
    z <<= b.z;
    w <<= b.w;
    return *this;
}

VectorInt4& VectorInt4::operator >>= (const VectorInt4& b)
{
    x >>= b.x;
    y >>= b.y;
    z >>= b.z;
    w >>= b.w;
    return *this;
}

const VectorInt4 VectorInt4::operator << (Int32 b) const
{
    return { x << b, y << b, z << b, w << b };
}

const VectorInt4 VectorInt4::operator >> (Int32 b) const
{
    return { x >> b, y >> b, z >> b, w >> b };
}

VectorInt4& VectorInt4::operator <<= (Int32 b)
{
    x <<= b;
    y <<= b;
    z <<= b;
    w <<= b;
    return *this;
}

VectorInt4& VectorInt4::operator >>= (Int32 b)
{
    x >>= b;
    y >>= b;
    z >>= b;
    w >>= b;
    return *this;
}

const VectorBool4 VectorInt4::operator == (const VectorInt4& b) const
{
    return VectorBool4{ x == b.x, y == b.y, z == b.z, w == b.w };
}

const VectorBool4 VectorInt4::operator != (const VectorInt4& b) const
{
    return VectorBool4{ x != b.x, y != b.y, z != b.z, w != b.w };
}

const VectorBool4 VectorInt4::operator < (const VectorInt4& b) const
{
    return VectorBool4{ x < b.x, y < b.y, z < b.z, w < b.w };
}

const VectorBool4 VectorInt4::operator > (const VectorInt4& b) const
{
    return VectorBool4{ x > b.x, y > b.y, z > b.z, w > b.w };
}

const VectorBool4 VectorInt4::operator >= (const VectorInt4& b) const
{
    return VectorBool4{ x >= b.x, y >= b.y, z >= b.z, w >= b.w };
}

const VectorBool4 VectorInt4::operator <= (const VectorInt4& b) const
{
    return VectorBool4{ x <= b.x, y <= b.y, z <= b.z, w <= b.w };
}

const VectorInt4 VectorInt4::Min(const VectorInt4& a, const VectorInt4& b)
{
    return
    {
        math::Min(a.x, b.x),
        math::Min(a.y, b.y),
        math::Min(a.z, b.z),
        math::Min(a.w, b.w)
    };
}

const VectorInt4 VectorInt4::Max(const VectorInt4& a, const VectorInt4& b)
{
    return
    {
        math::Max(a.x, b.x),
        math::Max(a.y, b.y),
        math::Max(a.z, b.z),
        math::Max(a.w, b.w)
    };
}


} // namespace math
} // namespace rt
