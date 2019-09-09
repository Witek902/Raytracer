#pragma once

namespace rt {
namespace math {

VectorBool8::VectorBool8(bool e0, bool e1, bool e2, bool e3, bool e4, bool e5, bool e6, bool e7)
{
    v = _mm256_castsi256_ps(_mm256_set_epi32(
        e7 ? 0xFFFFFFFF : 0,
        e6 ? 0xFFFFFFFF : 0,
        e5 ? 0xFFFFFFFF : 0,
        e4 ? 0xFFFFFFFF : 0,
        e3 ? 0xFFFFFFFF : 0,
        e2 ? 0xFFFFFFFF : 0,
        e1 ? 0xFFFFFFFF : 0,
        e0 ? 0xFFFFFFFF : 0
    ));
}

template<Uint32 index>
bool VectorBool8::Get() const
{
    static_assert(index < 8, "Invalid index");
    return _mm256_extract_epi32(_mm256_castps_si256(v), index) != 0;
}

int VectorBool8::GetMask() const
{
    return _mm256_movemask_ps(v);
}

bool VectorBool8::All() const
{
    return _mm256_movemask_ps(v) == 0xFF;
}

bool VectorBool8::None() const
{
    return _mm256_movemask_ps(v) == 0;
}

bool VectorBool8::Any() const
{
    return _mm256_movemask_ps(v) != 0;
}

const VectorBool8 VectorBool8::operator & (const VectorBool8 rhs) const
{
    return _mm256_and_ps(v, rhs.v);
}

const VectorBool8 VectorBool8::operator | (const VectorBool8 rhs) const
{
    return _mm256_or_ps(v, rhs.v);
}

const VectorBool8 VectorBool8::operator ^ (const VectorBool8 rhs) const
{
    return _mm256_xor_ps(v, rhs.v);
}

bool VectorBool8::operator == (const VectorBool8 rhs) const
{
    return GetMask() == rhs.GetMask();
}

} // namespace math
} // namespace rt
