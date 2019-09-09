#pragma once

namespace rt {
namespace math {

Vector8::Vector8()
    : low{}
    , high{}
{}

Vector8::Vector8(const Vector8& other)
    : low{ other.low }
    , high{ other.high }
{}

Vector8::Vector8(const Vector4& lo)
    : low{ lo }
    , high{ Vector4::Zero() }
{}

Vector8::Vector8(const Vector4& lo, const Vector4& hi)
    : low{ lo }
    , high{ hi }
{}

Vector8::Vector8(float e0, float e1, float e2, float e3, float e4, float e5, float e6, float e7)
    : f{ e0, e1, e2, e3, e4, e5, e6, e7 }
{}

Vector8::Vector8(Int32 e0, Int32 e1, Int32 e2, Int32 e3, Int32 e4, Int32 e5, Int32 e6, Int32 e7)
    : i{ e0, e1, e2, e3, e4, e5, e6, e7 }
{}

Vector8::Vector8(Uint32 e0, Uint32 e1, Uint32 e2, Uint32 e3, Uint32 e4, Uint32 e5, Uint32 e6, Uint32 e7)
    : u{ e0, e1, e2, e3, e4, e5, e6, e7 }
{}

Vector8::Vector8(const float* src)
    : low{ src }
    , high{ src + 4 }
{}

Vector8::Vector8(const float scalar)
    : low{ scalar }
    , high{ scalar }
{}

Vector8::Vector8(const Int32 i)
    : low{ i }
    , high{ i }
{}

Vector8::Vector8(const Uint32 u)
    : low{ u }
    , high{ u }
{}

const Vector8 Vector8::Zero()
{
    return { Vector4::Zero(), Vector4::Zero() };
}

const Vector8 Vector8::FromInteger(Int32 x)
{
    return { Vector4::FromInteger(x), Vector4::FromInteger(x) };
}

Vector8& Vector8::operator = (const Vector8& other)
{
    low = other.low;
    high = other.high;
    return *this;
}

const Vector8 Vector8::Select(const Vector8& a, const Vector8& b, const VectorBool8& sel)
{
    return
    {
        sel.Get<0>() ? b.f[0] : a.f[0],
        sel.Get<1>() ? b.f[1] : a.f[1],
        sel.Get<2>() ? b.f[2] : a.f[2],
        sel.Get<3>() ? b.f[3] : a.f[3],
        sel.Get<4>() ? b.f[3] : a.f[4],
        sel.Get<5>() ? b.f[4] : a.f[5],
        sel.Get<6>() ? b.f[5] : a.f[6],
        sel.Get<7>() ? b.f[6] : a.f[7],
    };
}

template<Uint32 selX, Uint32 selY, Uint32 selZ, Uint32 selW>
const Vector8 Vector8::Select(const Vector8& a, const Vector8& b)
{
    return { Vector4::Select<selX,selY,selZ,selW>(a.low, b.low), Vector4::Select<selX,selY,selZ,selW>(a.high, b.high) };
}

template<Uint32 ix, Uint32 iy, Uint32 iz, Uint32 iw>
const Vector8 Vector8::Swizzle() const
{
    static_assert(ix < 4, "Invalid X element index");
    static_assert(iy < 4, "Invalid Y element index");
    static_assert(iz < 4, "Invalid Z element index");
    static_assert(iw < 4, "Invalid W element index");

    return { low.Swizzle<ix, iy, iz, iw>(), high.Swizzle<ix, iy, iz, iw>() };
}

const Vector4 Vector8::Low() const
{
    return low;
}

const Vector4 Vector8::High() const
{
    return high;
}

const Vector8 Vector8::operator & (const Vector8& b) const
{
    return { low & b.low, high & b.high };
}

const Vector8 Vector8::operator | (const Vector8& b) const
{
    return { low | b.low, high | b.high };
}

const Vector8 Vector8::operator ^ (const Vector8& b) const
{
    return { low ^ b.low, high ^ b.high };
}

Vector8& Vector8::operator &= (const Vector8& b)
{
    low &= b.low;
    high &= b.high;
    return *this;
}

Vector8& Vector8::operator |= (const Vector8& b)
{
    low |= b.low;
    high |= b.high;
    return *this;
}

Vector8& Vector8::operator ^= (const Vector8& b)
{
    low ^= b.low;
    high ^= b.high;
    return *this;
}

const Vector8 Vector8::operator- () const
{
    return { -low, -high };
}

const Vector8 Vector8::operator + (const Vector8& b) const
{
    return { low + b.low, high + b.high };
}

const Vector8 Vector8::operator - (const Vector8& b) const
{
    return { low - b.low, high - b.high };
}

const Vector8 Vector8::operator * (const Vector8& b) const
{
    return { low * b.low, high * b.high };
}

const Vector8 Vector8::operator / (const Vector8& b) const
{
    return { low / b.low, high / b.high };
}

const Vector8 Vector8::operator * (float b) const
{
    return { low * b, high * b };
}

const Vector8 Vector8::operator / (float b) const
{
    return { low / b, high / b };
}

const Vector8 operator * (float a, const Vector8& b)
{
    return { a * b.low, a * b.high };
}

Vector8& Vector8::operator += (const Vector8& b)
{
    low += b.low;
    high += b.high;
    return *this;
}

Vector8& Vector8::operator -= (const Vector8& b)
{
    low -= b.low;
    high -= b.high;
    return *this;
}

Vector8& Vector8::operator *= (const Vector8& b)
{
    low *= b.low;
    high *= b.high;
    return *this;
}

Vector8& Vector8::operator /= (const Vector8& b)
{
    low /= b.low;
    high /= b.high;
    return *this;
}

Vector8& Vector8::operator *= (float b)
{
    low *= b;
    high *= b;
    return *this;
}

Vector8& Vector8::operator/= (float b)
{
    low /= b;
    high /= b;
    return *this;
}

const Vector8 Vector8::MulAndAdd(const Vector8& a, const Vector8& b, const Vector8& c)
{
    return a * b + c;
}

const Vector8 Vector8::MulAndSub(const Vector8& a, const Vector8& b, const Vector8& c)
{
    return a * b - c;
}

const Vector8 Vector8::NegMulAndAdd(const Vector8& a, const Vector8& b, const Vector8& c)
{
    return -(a * b) + c;
}

const Vector8 Vector8::NegMulAndSub(const Vector8& a, const Vector8& b, const Vector8& c)
{
    return c - a * b;
}

const Vector8 Vector8::Floor(const Vector8& v)
{
    return { Vector4::Floor(v.low), Vector4::Floor(v.high) };
}

const Vector8 Vector8::Sqrt(const Vector8& v)
{
    return { Vector4::Sqrt(v.low), Vector4::Sqrt(v.high) };
}

const Vector8 Vector8::Reciprocal(const Vector8& v)
{
    return { Vector4::Reciprocal(v.low), Vector4::Reciprocal(v.high) };
}

const Vector8 Vector8::FastReciprocal(const Vector8& v)
{
    return { Vector4::FastReciprocal(v.low), Vector4::FastReciprocal(v.high) };
}

const Vector8 Vector8::Min(const Vector8& a, const Vector8& b)
{
    return { Vector4::Min(a.low, b.low), Vector4::Min(a.high, b.high) };
}

const Vector8 Vector8::Max(const Vector8& a, const Vector8& b)
{
    return { Vector4::Max(a.low, b.low), Vector4::Max(a.high, b.high) };
}

const Vector8 Vector8::Abs(const Vector8& v)
{
    return { Vector4::Abs(v.low), Vector4::Abs(v.high) };
}

Int32 Vector8::GetSignMask() const
{
    return low.GetSignMask() | (high.GetSignMask() << 4);
}

const Vector8 Vector8::HorizontalMax() const
{
    const Vector4 max = Vector4::Max(low.HorizontalMax(), high.HorizontalMax());
    return { max, max };
}

const Vector8 Vector8::Fmod1(const Vector8& v)
{
    return { Vector4::Fmod1(v.low), Vector4::Fmod1(v.high) };
}

/*
void Vector8::Transpose8x8(Vector8& v0, Vector8& v1, Vector8& v2, Vector8& v3, Vector8& v4, Vector8& v5, Vector8& v6, Vector8& v7)
{
    // TODO
}
*/

const VectorBool8 Vector8::operator == (const Vector8& b) const
{
    return { low == b.low, high == b.high };
}

const VectorBool8 Vector8::operator < (const Vector8& b) const
{
    return { low < b.low, high < b.high };
}

const VectorBool8 Vector8::operator <= (const Vector8& b) const
{
    return { low <= b.low, high <= b.high };
}

const VectorBool8 Vector8::operator > (const Vector8& b) const
{
    return { low > b.low, high > b.high };
}

const VectorBool8 Vector8::operator >= (const Vector8& b) const
{
    return { low >= b.low, high >= b.high };
}

const VectorBool8 Vector8::operator != (const Vector8& b) const
{
    return { low != b.low, high != b.high };
}

bool Vector8::IsZero() const
{
    return low.IsZero().All() && high.IsZero().All();
}

bool Vector8::IsNaN() const
{
    return low.IsNaN().All() || high.IsNaN().All();
}

bool Vector8::IsInfinite() const
{
    return low.IsInfinite().All() || high.IsInfinite().All();
}

} // namespace math
} // namespace rt
