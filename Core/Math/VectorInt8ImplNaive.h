#pragma once

#include "Vector8.h"

namespace rt {
namespace math {

const VectorInt8 VectorInt8::Zero()
{
    return VectorInt8{ 0 };
}

VectorInt8::VectorInt8(const VectorInt8& other)
    : low{ other.low }
    , high{ other.high }
{}

VectorInt8& VectorInt8::operator = (const VectorInt8& other)
{
    low = other.low;
    high = other.high;
    return *this;
}

VectorInt8::VectorInt8(const VectorInt4& lo, const VectorInt4& hi)
    : low{ lo }, high{ hi }
{}

const VectorInt8 VectorInt8::Cast(const Vector8& v)
{
    return reinterpret_cast<const VectorInt8&>(v);
}

const Vector8 VectorInt8::CastToFloat() const
{
    return reinterpret_cast<const Vector8&>(*this);
}

VectorInt8::VectorInt8(const Int32 e0, const Int32 e1, const Int32 e2, const Int32 e3, const Int32 e4, const Int32 e5, const Int32 e6, const Int32 e7)
    : i{ e0, e1, e2, e3, e4, e5, e6, e7 }
{}

VectorInt8::VectorInt8(const Int32 i)
    : low{ i }
    , high{ i }
{}

VectorInt8::VectorInt8(const Uint32 u)
    : low{ u }
    , high{ u }
{}

const VectorInt8 VectorInt8::operator & (const VectorInt8& b) const
{
    return { low & b.low, high & b.high };
}

const VectorInt8 VectorInt8::operator | (const VectorInt8& b) const
{
    return { low | b.low, high | b.high };
}

const VectorInt8 VectorInt8::operator ^ (const VectorInt8& b) const
{
    return { low ^ b.low, high ^ b.high };
}

VectorInt8& VectorInt8::operator &= (const VectorInt8& b)
{
    low &= b.low;
    high &= b.high;
    return *this;
}

VectorInt8& VectorInt8::operator |= (const VectorInt8& b)
{
    low |= b.low;
    high |= b.high;
    return *this;
}

VectorInt8& VectorInt8::operator ^= (const VectorInt8& b)
{
    low ^= b.low;
    high ^= b.high;
    return *this;
}

const VectorInt8 VectorInt8::Convert(const Vector8& v)
{
    return { VectorInt4::Convert(v.low), VectorInt4::Convert(v.high) };
}

const Vector8 VectorInt8::ConvertToFloat() const
{
    return { low.ConvertToFloat(), high.ConvertToFloat() };
}

const VectorInt8 VectorInt8::operator - () const
{
    return VectorInt8::Zero() - (*this);
}

const VectorInt8 VectorInt8::operator + (const VectorInt8& b) const
{
    return { low + b.low, high + b.high };
}

const VectorInt8 VectorInt8::operator - (const VectorInt8& b) const
{
    return { low - b.low, high - b.high };
}

const VectorInt8 VectorInt8::operator * (const VectorInt8& b) const
{
    return { low * b.low, high * b.high };
}

VectorInt8& VectorInt8::operator += (const VectorInt8& b)
{
    low += b.low;
    high += b.high;
    return *this;
}

VectorInt8& VectorInt8::operator -= (const VectorInt8& b)
{
    low -= b.low;
    high -= b.high;
    return *this;
}

VectorInt8& VectorInt8::operator *= (const VectorInt8& b)
{
    low *= b.low;
    high *= b.high;
    return *this;
}

const VectorInt8 VectorInt8::operator + (Int32 b) const
{
    return { low + b, high + b };
}

const VectorInt8 VectorInt8::operator - (Int32 b) const
{
    return { low - b, high - b };
}

const VectorInt8 VectorInt8::operator * (Int32 b) const
{
    return { low * b, high * b };
}

const VectorInt8 VectorInt8::operator % (Int32 b) const
{
    // TODO
    return VectorInt8(i[0] % b, i[1] % b, i[2] % b, i[3] % b, i[4] % b, i[5] % b, i[6] % b, i[7] % b);
}

VectorInt8& VectorInt8::operator += (Int32 b)
{
    low += b;
    high += b;
    return *this;
}

VectorInt8& VectorInt8::operator -= (Int32 b)
{
    low -= b;
    high -= b;
    return *this;
}

VectorInt8& VectorInt8::operator *= (Int32 b)
{
    low *= b;
    high *= b;
    return *this;
}

bool VectorInt8::operator == (const VectorInt8& b) const
{
    return (low == b.low).All() && (high == b.high).All();
}

bool VectorInt8::operator != (const VectorInt8& b) const
{
    return (low != b.low).Any() || (high != b.high).Any();
}

const VectorInt8 VectorInt8::operator << (const VectorInt8& b) const
{
    return
    {
        i[0] << b.i[0],
        i[1] << b.i[1],
        i[2] << b.i[2],
        i[3] << b.i[3],
        i[4] << b.i[4],
        i[5] << b.i[5],
        i[6] << b.i[6],
        i[7] << b.i[7]
    };
}

const VectorInt8 VectorInt8::operator >> (const VectorInt8& b) const
{
    return
    {
        i[0] >> b.i[0],
        i[1] >> b.i[1],
        i[2] >> b.i[2],
        i[3] >> b.i[3],
        i[4] >> b.i[4],
        i[5] >> b.i[5],
        i[6] >> b.i[6],
        i[7] >> b.i[7]
    };
}

const VectorInt8 VectorInt8::operator << (Int32 b) const
{
    return { low << b, high << b };
}

const VectorInt8 VectorInt8::operator >> (Int32 b) const
{
    return { low >> b, high >> b };
}

const VectorInt8 VectorInt8::Min(const VectorInt8& a, const VectorInt8& b)
{
    return { VectorInt4::Min(a.low, b.low), VectorInt4::Min(a.high, b.high) };
}

const VectorInt8 VectorInt8::Max(const VectorInt8& a, const VectorInt8& b)
{
    return { VectorInt4::Max(a.low, b.low), VectorInt4::Max(a.high, b.high) };
}

const Vector8 Gather8(const float* basePtr, const VectorInt8& indices)
{
    Vector8 result;
    for (Uint32 i = 0; i < 8; ++i)
    {
        result[i] = basePtr[indices[i]];
    }
    return result;
}

} // namespace math
} // namespace rt
