#pragma once

#include "Math.h"

namespace rt {
namespace math {

/**
 * Structure for efficient (in terms of memory) 2D vector storing.
 */
struct Float2
{
    union
    {
        float f[2];

        struct
        {
            float x;
            float y;
        };
    };

    RT_INLINE constexpr Float2();
    RT_INLINE constexpr explicit Float2(float v);
    RT_INLINE constexpr Float2(float x, float y);
    RT_INLINE constexpr explicit Float2(const float* src);

    /// element access
    RT_INLINE float Get(Uint32 index) const;
    RT_INLINE float& Get(Uint32 index);

    /// elements manipulations
    RT_INLINE constexpr Float2 SplatX() const;
    RT_INLINE constexpr Float2 SplatY() const;
    RT_INLINE constexpr static Float2 Splat(float f);

    template<bool x, bool y>
    RT_INLINE constexpr Float2 ChangeSign() const;

    template<Uint32 ix, Uint32 iy>
    RT_INLINE Float2 Swizzle() const;

    template<Uint32 ix, Uint32 iy>
    RT_INLINE static constexpr Float2 Blend(const Float2& a, const Float2& b);

    RT_INLINE static Float2 SelectBySign(const Float2& a, const Float2& b, const Float2& sel);

    /// simple arithmetics
    RT_INLINE constexpr Float2 operator- () const;
    RT_INLINE constexpr Float2 operator+ (const Float2& b) const;
    RT_INLINE constexpr Float2 operator- (const Float2& b) const;
    RT_INLINE constexpr Float2 operator* (const Float2& b) const;
    RT_INLINE Float2 operator/ (const Float2& b) const;
    RT_INLINE constexpr Float2 operator* (float b) const;
    RT_INLINE Float2 operator/ (float b) const;
    RT_INLINE Float2& operator+= (const Float2& b);
    RT_INLINE Float2& operator-= (const Float2& b);
    RT_INLINE Float2& operator*= (const Float2& b);
    RT_INLINE Float2& operator/= (const Float2& b);
    RT_INLINE Float2& operator*= (float b);
    RT_INLINE Float2& operator/= (float b);

    /// comparison operators (returns true, if all the elements satisfy the equation)
    RT_INLINE constexpr bool operator== (const Float2& b) const;
    RT_INLINE constexpr bool operator< (const Float2& b) const;
    RT_INLINE constexpr bool operator<= (const Float2& b) const;
    RT_INLINE constexpr bool operator> (const Float2& b) const;
    RT_INLINE constexpr bool operator>= (const Float2& b) const;
    RT_INLINE constexpr bool operator!= (const Float2& b) const;

    /// Misc math
    RT_INLINE static Float2 Floor(const Float2& v);
    RT_INLINE static Float2 Sqrt(const Float2& v);
    RT_INLINE static Float2 Reciprocal(const Float2& v);
    RT_INLINE static constexpr Float2 Lerp(const Float2& v1, const Float2& v2, const Float2& weight);
    RT_INLINE static constexpr Float2 Lerp(const Float2& v1, const Float2& v2, float weight);
    RT_INLINE static constexpr Float2 Min(const Float2& a, const Float2& b);
    RT_INLINE static constexpr Float2 Max(const Float2& a, const Float2& b);
    RT_INLINE static constexpr Float2 Abs(const Float2& v);
    RT_INLINE static constexpr bool AlmostEqual(const Float2& v1, const Float2& v2, float epsilon = FLT_EPSILON);

    /// Geometry
    RT_INLINE static constexpr float Dot(const Float2& a, const Float2& b);
    RT_INLINE static constexpr float Cross(const Float2& a, const Float2& b);
    RT_INLINE float Length() const;
    RT_INLINE Float2& Normalize();
    RT_INLINE Float2 Normalized() const;
};

RT_INLINE Float2 operator * (float a, const Float2& b);


} // namespace math
} // namespace rt


#include "Float2Impl.h"
