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

    RT_FORCE_INLINE explicit VectorBool4(bool x, bool y, bool z, bool w);
    RT_FORCE_INLINE explicit VectorBool4(int x, int y, int z, int w);

    template<Uint32 index>
    RT_FORCE_INLINE bool Get() const;

    template<Uint32 ix, Uint32 iy, Uint32 iz, Uint32 iw>
    RT_FORCE_INLINE const VectorBool4 Swizzle() const;

    // combine into 4-bit mask
    RT_FORCE_INLINE Int32 GetMask() const;

    RT_FORCE_INLINE bool All() const;
    RT_FORCE_INLINE bool None() const;
    RT_FORCE_INLINE bool Any() const;

    RT_FORCE_INLINE const VectorBool4 operator & (const VectorBool4 rhs) const;
    RT_FORCE_INLINE const VectorBool4 operator | (const VectorBool4 rhs) const;
    RT_FORCE_INLINE const VectorBool4 operator ^ (const VectorBool4 rhs) const;

    RT_FORCE_INLINE bool operator == (const VectorBool4 rhs) const;

private:
    friend struct Vector4;
    friend struct VectorInt4;
    friend struct VectorBool8;

#ifdef RT_USE_SSE
    RT_FORCE_INLINE VectorBool4(const __m128 other) : v(other) { }
    RT_FORCE_INLINE VectorBool4(const __m128i other) : v(_mm_castsi128_ps(other)) { }
    RT_FORCE_INLINE operator __m128() const { return v; }
    RT_FORCE_INLINE operator __m128i() const { return _mm_castps_si128(v); }

    __m128 v;
#else
    bool b[4];
#endif // RT_USE_SSE
};

} // namespace math
} // namespace rt


#ifdef RT_USE_SSE
#include "VectorBool4ImplSSE.h"
#else
#include "VectorBool4ImplNaive.h"
#endif
