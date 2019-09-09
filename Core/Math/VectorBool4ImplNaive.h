#pragma once

namespace rt {
namespace math {

VectorBool4::VectorBool4(bool x, bool y, bool z, bool w)
    : b{ x, y, z, w }
{}

VectorBool4::VectorBool4(int x, int y, int z, int w)
    : b{ x > 0, y > 0, z > 0, w > 0 }
{}

template<Uint32 index>
bool VectorBool4::Get() const
{
    static_assert(index < 4, "Invalid index");
    return b[index];
}

template<Uint32 ix, Uint32 iy, Uint32 iz, Uint32 iw>
const VectorBool4 VectorBool4::Swizzle() const
{
    static_assert(ix < 4, "Invalid X element index");
    static_assert(iy < 4, "Invalid Y element index");
    static_assert(iz < 4, "Invalid Z element index");
    static_assert(iw < 4, "Invalid W element index");

    return VectorBool4{ b[ix], b[iy], b[iz], b[iw] };
}

// combine into 4-bit mask
Int32 VectorBool4::GetMask() const
{
    Int32 ret = 0;
    ret |= b[0] ? (1 << 0) : 0;
    ret |= b[1] ? (1 << 1) : 0;
    ret |= b[2] ? (1 << 2) : 0;
    ret |= b[3] ? (1 << 3) : 0;
    return ret;
}

bool VectorBool4::All() const
{
    return b[0] && b[1] && b[2] && b[3];
}

bool VectorBool4::None() const
{
    return (!b[0]) && (!b[1]) && (!b[2]) && (!b[3]);
}

bool VectorBool4::Any() const
{
    return b[0] || b[1] || b[2] || b[3];
}

const VectorBool4 VectorBool4::operator & (const VectorBool4 rhs) const
{
    return VectorBool4{ b[0] && rhs.b[0], b[1] && rhs.b[1], b[2] && rhs.b[2], b[3] && rhs.b[3] };
}

const VectorBool4 VectorBool4::operator | (const VectorBool4 rhs) const
{
    return VectorBool4{ b[0] || rhs.b[0], b[1] || rhs.b[1], b[2] || rhs.b[2], b[3] || rhs.b[3] };
}

const VectorBool4 VectorBool4::operator ^ (const VectorBool4 rhs) const
{
    return VectorBool4{ b[0] ^ rhs.b[0], b[1] ^ rhs.b[1], b[2] ^ rhs.b[2], b[3] ^ rhs.b[3] };
}

bool VectorBool4::operator == (const VectorBool4 rhs) const
{
    return (b[0] == rhs.b[0]) && (b[1] == rhs.b[1]) && (b[2] == rhs.b[2]) && (b[3] == rhs.b[3]);
}

} // namespace math
} // namespace rt
