#pragma once

namespace rt {
namespace math {

VectorBool8::VectorBool8(bool e0, bool e1, bool e2, bool e3, bool e4, bool e5, bool e6, bool e7)
    : b{ e0, e1, e2, e3, e4, e5, e6, e7 }
{}

VectorBool8::VectorBool8(const VectorBool4& low, const VectorBool4& high)
    : b{ low.Get<0>(), low.Get<1>(), low.Get<2>(), low.Get<3>(),
         high.Get<0>(), high.Get<1>(), high.Get<2>(), high.Get<3>() }
{}

template<Uint32 index>
bool VectorBool8::Get() const
{
    static_assert(index < 8, "Invalid index");
    return b[index];
}

int VectorBool8::GetMask() const
{
    Int32 ret = 0;
    for (Uint32 i = 0; i < 8; ++i)
    {
        ret |= b[i] ? (1 << i) : 0;
    }
    return ret;
}

bool VectorBool8::All() const
{
    bool ret = true;
    for (Uint32 i = 0; i < 8; ++i)
    {
        ret &= b[i];
    }
    return ret;
}

bool VectorBool8::None() const
{
    bool ret = true;
    for (Uint32 i = 0; i < 8; ++i)
    {
        ret &= !(b[i]);
    }
    return ret;
}

bool VectorBool8::Any() const
{
    bool ret = false;
    for (Uint32 i = 0; i < 8; ++i)
    {
        ret |= (b[i]);
    }
    return ret;
}

const VectorBool8 VectorBool8::operator & (const VectorBool8 rhs) const
{
    VectorBool8 ret;
    for (Uint32 i = 0; i < 8; ++i)
    {
        ret.b[i] = b[i] && rhs.b[i];
    }
    return ret;
}

const VectorBool8 VectorBool8::operator | (const VectorBool8 rhs) const
{
    VectorBool8 ret;
    for (Uint32 i = 0; i < 8; ++i)
    {
        ret.b[i] = b[i] || rhs.b[i];
    }
    return ret;
}

const VectorBool8 VectorBool8::operator ^ (const VectorBool8 rhs) const
{
    VectorBool8 ret;
    for (Uint32 i = 0; i < 8; ++i)
    {
        ret.b[i] = b[i] ^ rhs.b[i];
    }
    return ret;
}

bool VectorBool8::operator == (const VectorBool8 rhs) const
{
    bool ret = true;
    for (Uint32 i = 0; i < 8; ++i)
    {
        ret &= b[i] == rhs.b[i];
    }
    return ret;
}

} // namespace math
} // namespace rt
