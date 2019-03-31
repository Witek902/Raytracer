#pragma once

#include "Math.h"

namespace rt {
namespace math {

/**
 * 4-element boolean vector
 */
struct RT_ALIGN(16) VectorBool4
{
    VectorBool4() = default;

    RT_FORCE_INLINE explicit VectorBool4(bool x, bool y, bool z, bool w)
    {
        v = _mm_castsi128_ps(_mm_set_epi32(w ? 0xFFFFFFFF : 0, z ? 0xFFFFFFFF : 0, y ? 0xFFFFFFFF : 0, x ? 0xFFFFFFFF : 0));
    }

    RT_FORCE_INLINE explicit VectorBool4(int x, int y, int z, int w)
    {
        v = _mm_castsi128_ps(_mm_set_epi32((w > 0) ? 0xFFFFFFFF : 0, (z > 0) ? 0xFFFFFFFF : 0, (y > 0) ? 0xFFFFFFFF : 0, (x > 0) ? 0xFFFFFFFF : 0));
    }

    template<Uint32 index>
    RT_FORCE_INLINE bool Get() const
    {
        static_assert(index < 4, "Invalid index");
        return _mm_extract_ps(v, index) != 0;
    }

    // combine into 4-bit mask
    RT_FORCE_INLINE int GetMask() const
    {
        return _mm_movemask_ps(v);
    }

    RT_FORCE_INLINE bool All() const
    {
        return _mm_movemask_ps(v) == 0xF;
    }

    RT_FORCE_INLINE bool None() const
    {
        return _mm_movemask_ps(v) == 0;
    }

    RT_FORCE_INLINE bool Any() const
    {
        return _mm_movemask_ps(v) != 0;
    }

    const VectorBool4 operator & (const VectorBool4 rhs) const
    {
        return _mm_and_ps(v, rhs.v);
    }

    const VectorBool4 operator | (const VectorBool4 rhs) const
    {
        return _mm_or_ps(v, rhs.v);
    }

    const VectorBool4 operator ^ (const VectorBool4 rhs) const
    {
        return _mm_xor_ps(v, rhs.v);
    }

    bool operator == (const VectorBool4 rhs) const
    {
        return GetMask() == rhs.GetMask();
    }

private:
    friend struct Vector4;
    friend struct VectorInt4;

    RT_FORCE_INLINE VectorBool4(const __m128 other) : v(other) { }
    RT_FORCE_INLINE VectorBool4(const __m128i other) : v(_mm_castsi128_ps(other)) { }

    __m128 v;
};

} // namespace math
} // namespace rt
