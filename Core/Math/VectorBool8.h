#pragma once

#include "Math.h"

namespace rt {
namespace math {

/**
 * 8-element boolean vector
 */
struct RT_ALIGN(32) VectorBool8
{
    VectorBool8() = default;

    RT_FORCE_INLINE VectorBool8(bool e0, bool e1, bool e2, bool e3, bool e4, bool e5, bool e6, bool e7)
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

    RT_FORCE_INLINE VectorBool8(const __m256 other) : v(other) { }

    template<Uint32 index>
    RT_FORCE_INLINE bool Get() const
    {
        static_assert(index < 8, "Invalid index");
        return _mm_extract_ps(v, index) != 0;
    }

    // combine into 8-bit mask
    RT_FORCE_INLINE int GetMask() const
    {
        return _mm256_movemask_ps(v);
    }

    RT_FORCE_INLINE bool All() const
    {
        return _mm256_movemask_ps(v) == 0xFF;
    }

    RT_FORCE_INLINE bool None() const
    {
        return _mm256_movemask_ps(v) == 0;
    }

    RT_FORCE_INLINE bool Any() const
    {
        return _mm256_movemask_ps(v) != 0;
    }

    const VectorBool8 operator & (const VectorBool8 rhs) const
    {
        return _mm256_and_ps(v, rhs.v);
    }

    const VectorBool8 operator | (const VectorBool8 rhs) const
    {
        return _mm256_or_ps(v, rhs.v);
    }

    const VectorBool8 operator ^ (const VectorBool8 rhs) const
    {
        return _mm256_xor_ps(v, rhs.v);
    }

    bool operator == (const VectorBool8 rhs) const
    {
        return GetMask() == rhs.GetMask();
    }

private:
    friend struct Vector8;

    __m256 v;
};

} // namespace math
} // namespace rt
