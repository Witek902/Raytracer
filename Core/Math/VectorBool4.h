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

    template<Uint32 ix, Uint32 iy, Uint32 iz, Uint32 iw>
    const VectorBool4 Swizzle() const
    {
        static_assert(ix < 4, "Invalid X element index");
        static_assert(iy < 4, "Invalid Y element index");
        static_assert(iz < 4, "Invalid Z element index");
        static_assert(iw < 4, "Invalid W element index");

        if (ix == 0 && iy == 0 && iz == 1 && iw == 1)
        {
            return _mm_unpacklo_ps(v, v);
        }
        else if (ix == 2 && iy == 2 && iz == 3 && iw == 3)
        {
            return _mm_unpackhi_ps(v, v);
        }
        else if (ix == 0 && iy == 1 && iz == 0 && iw == 1)
        {
            return _mm_movelh_ps(v, v);
        }
        else if (ix == 2 && iy == 3 && iz == 2 && iw == 3)
        {
            return _mm_movehl_ps(v, v);
        }
        else if (ix == 0 && iy == 0 && iz == 2 && iw == 2)
        {
            return _mm_moveldup_ps(v);
        }
        else if (ix == 1 && iy == 1 && iz == 3 && iw == 3)
        {
            return _mm_movehdup_ps(v);
        }

        return _mm_shuffle_ps(v, v, _MM_SHUFFLE(iw, iz, iy, ix));
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
