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

    RT_FORCE_INLINE VectorBool8(bool e0, bool e1, bool e2, bool e3, bool e4, bool e5, bool e6, bool e7);
    RT_FORCE_INLINE VectorBool8(const VectorBool4& low, const VectorBool4& high);

#ifdef RT_USE_AVX
    RT_FORCE_INLINE VectorBool8(const __m256 other) : v(other) { }
    RT_FORCE_INLINE VectorBool8(const __m256i other) : v(_mm256_castsi256_ps(other)) { }
    RT_FORCE_INLINE operator __m256() const { return v; }
    RT_FORCE_INLINE operator __m256i() const { return _mm256_castps_si256(v); }
#endif // RT_USE_SSE

    template<uint32 index>
    RT_FORCE_INLINE bool Get() const;

    // combine into 8-bit mask
    RT_FORCE_INLINE int GetMask() const;

    RT_FORCE_INLINE bool All() const;
    RT_FORCE_INLINE bool None() const;
    RT_FORCE_INLINE bool Any() const;

    RT_FORCE_INLINE const VectorBool8 operator & (const VectorBool8 rhs) const;
    RT_FORCE_INLINE const VectorBool8 operator | (const VectorBool8 rhs) const;
    RT_FORCE_INLINE const VectorBool8 operator ^ (const VectorBool8 rhs) const;

    RT_FORCE_INLINE bool operator == (const VectorBool8 rhs) const;

private:
    friend struct Vector8;
    friend struct VectorInt8;

#ifdef RT_USE_AVX
    __m256 v;
#else
    bool b[8];
#endif // RT_USE_SSE
};

} // namespace math
} // namespace rt


#ifdef RT_USE_AVX
#include "VectorBool8ImplAVX.h"
#else
#include "VectorBool8ImplNaive.h"
#endif
