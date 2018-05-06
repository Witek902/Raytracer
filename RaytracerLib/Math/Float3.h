#pragma once

#include "Math.h"
#include "Float2.h"

namespace rt {
namespace math {

/**
 * Structure for efficient (in terms of memory) 3D vector storing.
 */
struct Float3
{
    union
    {
        float f[3];

        struct
        {
            float x;
            float y;
            float z;
        };
    };

    RT_INLINE constexpr Float3();
    RT_INLINE constexpr Float3(const Float2& rhs);
    RT_INLINE constexpr explicit Float3(float x, float y = 0.0f, float z = 0.0f);
    RT_INLINE constexpr explicit Float3(const float* src);

    // cast to Float2 (Z component is discarded)
    RT_INLINE operator Float2() const;

    /// element access
    RT_INLINE float Get(Uint32 index) const;
    RT_INLINE float& Get(Uint32 index);

    /// elements manipulations
    RT_INLINE constexpr Float3 SplatX() const;
    RT_INLINE constexpr Float3 SplatY() const;
    RT_INLINE constexpr Float3 SplatZ() const;
    RT_INLINE constexpr static Float3 Splat(float f);

    template<bool x, bool y, bool z>
    RT_INLINE constexpr Float3 ChangeSign() const;

    template<Uint32 ix, Uint32 iy, Uint32 iz>
    RT_INLINE Float3 Swizzle() const;

    template<Uint32 ix, Uint32 iy, Uint32 iz>
    RT_INLINE static constexpr Float3 Blend(const Float3& a, const Float3& b);

    RT_INLINE static Float3 SelectBySign(const Float3& a, const Float3& b, const Float3& sel);

    /// simple arithmetics
    RT_INLINE constexpr Float3 operator- () const;
    RT_INLINE constexpr Float3 operator+ (const Float3& b) const;
    RT_INLINE constexpr Float3 operator- (const Float3& b) const;
    RT_INLINE constexpr Float3 operator* (const Float3& b) const;
    RT_INLINE Float3 operator/ (const Float3& b) const;
    RT_INLINE constexpr Float3 operator* (float b) const;
    RT_INLINE Float3 operator/ (float b) const;
    RT_INLINE Float3& operator+= (const Float3& b);
    RT_INLINE Float3& operator-= (const Float3& b);
    RT_INLINE Float3& operator*= (const Float3& b);
    RT_INLINE Float3& operator/= (const Float3& b);
    RT_INLINE Float3& operator*= (float b);
    RT_INLINE Float3& operator/= (float b);

    /// comparison operators (returns true, if all the elements satisfy the equation)
    RT_INLINE constexpr bool operator== (const Float3& b) const;
    RT_INLINE constexpr bool operator< (const Float3& b) const;
    RT_INLINE constexpr bool operator<= (const Float3& b) const;
    RT_INLINE constexpr bool operator> (const Float3& b) const;
    RT_INLINE constexpr bool operator>= (const Float3& b) const;
    RT_INLINE constexpr bool operator!= (const Float3& b) const;

    /// Misc math
    RT_INLINE static Float3 Floor(const Float3& v);
    RT_INLINE static Float3 Sqrt(const Float3& v);
    RT_INLINE static Float3 Reciprocal(const Float3& v);
    RT_INLINE static constexpr Float3 Lerp(const Float3& v1, const Float3& v2, const Float3& weight);
    RT_INLINE static constexpr Float3 Lerp(const Float3& v1, const Float3& v2, float weight);
    RT_INLINE static constexpr Float3 Min(const Float3& a, const Float3& b);
    RT_INLINE static constexpr Float3 Max(const Float3& a, const Float3& b);
    RT_INLINE static constexpr Float3 Abs(const Float3& v);
    RT_INLINE static constexpr bool AlmostEqual(const Float3& v1, const Float3& v2, float epsilon = FLT_EPSILON);

    /// Geometry
    RT_INLINE static constexpr float Dot(const Float3& a, const Float3& b);
    RT_INLINE static constexpr Float3 Cross(const Float3& a, const Float3& b);
    RT_INLINE float Length() const;
    RT_INLINE Float3& Normalize();
    RT_INLINE Float3 Normalized() const;
};

RT_INLINE Float3 operator * (float a, const Float3& b);

} // namespace math
} // namespace rt

#include "Float3Impl.h"
