#pragma once

#include "Math.h"

#include "Float2.h"
#include "Float3.h"


#define RT_USE_FMA

namespace rt {
namespace math {

/**
 * 4-element SIMD vector.
 */
struct RT_ALIGN(16) Vector4
{
    union
    {
        Float f[4];
        Int32 i[4];
        Uint32 u[4];
        __m128 v;

        struct
        {
            float x;
            float y;
            float z;
            float w;
        };
    };

    RT_FORCE_INLINE Vector4(const __m128& src)
    {
        v = src;
    }

    // constructors
    RT_FORCE_INLINE Vector4();
    RT_FORCE_INLINE explicit Vector4(const Float s); // splat
    RT_FORCE_INLINE Vector4(const Float x, const Float y, const Float z, const Float w);
    RT_FORCE_INLINE Vector4(const Int32 x, const Int32 y, const Int32 z, const Int32 w);
    RT_FORCE_INLINE Vector4(const Uint32 x, const Uint32 y, const Uint32 z, const Uint32 w);
    RT_FORCE_INLINE explicit Vector4(const Float* src);
    RT_FORCE_INLINE explicit Vector4(const Float2& src);
    RT_FORCE_INLINE explicit Vector4(const Float3& src);
    RT_FORCE_INLINE void Set(Float scalar);
    RT_FORCE_INLINE static Vector4 FromIntegers(Uint32 x, Uint32 y = 0, Uint32 z = 0, Uint32 w = 0);

    RT_FORCE_INLINE operator __m128() const { return v; }
    RT_FORCE_INLINE operator __m128i() const { return reinterpret_cast<const __m128i*>(&v)[0]; }
    RT_FORCE_INLINE Float operator[] (Uint32 index) const { return f[index]; }
    RT_FORCE_INLINE Float& operator[] (Uint32 index) { return f[index]; }

    /// simple arithmetics
    RT_FORCE_INLINE Vector4 operator- () const;
    RT_FORCE_INLINE Vector4 operator+ (const Vector4& b) const;
    RT_FORCE_INLINE Vector4 operator- (const Vector4& b) const;
    RT_FORCE_INLINE Vector4 operator* (const Vector4& b) const;
    RT_FORCE_INLINE Vector4 operator/ (const Vector4& b) const;
    RT_FORCE_INLINE Vector4 operator* (Float b) const;
    RT_FORCE_INLINE Vector4 operator/ (Float b) const;
    RT_FORCE_INLINE Vector4& operator+= (const Vector4& b);
    RT_FORCE_INLINE Vector4& operator-= (const Vector4& b);
    RT_FORCE_INLINE Vector4& operator*= (const Vector4& b);
    RT_FORCE_INLINE Vector4& operator/= (const Vector4& b);
    RT_FORCE_INLINE Vector4& operator*= (Float b);
    RT_FORCE_INLINE Vector4& operator/= (Float b);

    /// comparison operators (returns true, if all the elements satisfy the equation)
    RT_FORCE_INLINE bool operator== (const Vector4& b) const;
    RT_FORCE_INLINE bool operator< (const Vector4& b) const;
    RT_FORCE_INLINE bool operator<= (const Vector4& b) const;
    RT_FORCE_INLINE bool operator> (const Vector4& b) const;
    RT_FORCE_INLINE bool operator>= (const Vector4& b) const;
    RT_FORCE_INLINE bool operator!= (const Vector4& b) const;

    /// bitwise logic operations
    RT_FORCE_INLINE Vector4 operator& (const Vector4& b) const;
    RT_FORCE_INLINE Vector4 operator| (const Vector4& b) const;
    RT_FORCE_INLINE Vector4 operator^ (const Vector4& b) const;
    RT_FORCE_INLINE Vector4& operator&= (const Vector4& b);
    RT_FORCE_INLINE Vector4& operator|= (const Vector4& b);
    RT_FORCE_INLINE Vector4& operator^= (const Vector4& b);

    RT_FORCE_INLINE Vector4 SplatX() const;
    RT_FORCE_INLINE Vector4 SplatY() const;
    RT_FORCE_INLINE Vector4 SplatZ() const;
    RT_FORCE_INLINE Vector4 SplatW() const;

    /**
     * Rearrange vector elements.
     */
    template<bool x = false, bool y = false, bool z = false, bool w = false>
    RT_FORCE_INLINE Vector4 ChangeSign() const;

    /**
     * Rearrange vector elements.
     */
    template<Uint32 ix = 0, Uint32 iy = 1, Uint32 iz = 2, Uint32 iw = 3>
    RT_FORCE_INLINE Vector4 Swizzle() const;

    /**
     * Convert 4 uint8 to a Vector4.
     */
    RT_FORCE_INLINE static Vector4 Load4(const Uint8* src);

	/**
     * Convert 3 uint8 to a Vector4 and scale to 0...1 range.
     */
	RT_FORCE_INLINE static Vector4 LoadBGR_UNorm(const Uint8* src);

    /**
     * Convert a Vector4 to 4 unsigned chars.
     */
    RT_FORCE_INLINE void Store4_NonTemporal(Uint8* dest) const;

    RT_FORCE_INLINE void Store(Float* dest) const;
    RT_FORCE_INLINE void Store(Float2* dest) const;
    RT_FORCE_INLINE void Store(Float3* dest) const;
    RT_FORCE_INLINE Float3 ToFloat3() const;

    RT_FORCE_INLINE static Vector4 Floor(const Vector4& v);
    RT_FORCE_INLINE static Vector4 Sqrt(const Vector4& v);
    RT_FORCE_INLINE static Vector4 Sqrt4(const Vector4& v);
    RT_FORCE_INLINE static Vector4 Reciprocal(const Vector4& v);
    RT_FORCE_INLINE static Vector4 FastReciprocal(const Vector4& v);
    RT_FORCE_INLINE static Vector4 Lerp(const Vector4& v1, const Vector4& v2, const Vector4& weight);
    RT_FORCE_INLINE static Vector4 Lerp(const Vector4& v1, const Vector4& v2, Float weight);
    RT_FORCE_INLINE static Vector4 Min(const Vector4& a, const Vector4& b);
    RT_FORCE_INLINE static Vector4 Max(const Vector4& a, const Vector4& b);
    RT_FORCE_INLINE static Vector4 Abs(const Vector4& v);
    RT_FORCE_INLINE Vector4 Clamped(const Vector4& min, const Vector4& max) const;

    RT_FORCE_INLINE static int EqualMask(const Vector4& v1, const Vector4& v2);
    RT_FORCE_INLINE static int LessMask(const Vector4& v1, const Vector4& v2);
    RT_FORCE_INLINE static int LessEqMask(const Vector4& v1, const Vector4& v2);
    RT_FORCE_INLINE static int GreaterMask(const Vector4& v1, const Vector4& v2);
    RT_FORCE_INLINE static int GreaterEqMask(const Vector4& v1, const Vector4& v2);
    RT_FORCE_INLINE static int NotEqualMask(const Vector4& v1, const Vector4& v2);

    RT_FORCE_INLINE static bool Equal2(const Vector4& v1, const Vector4& v2);
    RT_FORCE_INLINE static bool Less2(const Vector4& v1, const Vector4& v2);
    RT_FORCE_INLINE static bool LessEq2(const Vector4& v1, const Vector4& v2);
    RT_FORCE_INLINE static bool Greater2(const Vector4& v1, const Vector4& v2);
    RT_FORCE_INLINE static bool GreaterEq2(const Vector4& v1, const Vector4& v2);
    RT_FORCE_INLINE static bool NotEqual2(const Vector4& v1, const Vector4& v2);

    RT_FORCE_INLINE static bool Equal3(const Vector4& v1, const Vector4& v2);
    RT_FORCE_INLINE static bool Less3(const Vector4& v1, const Vector4& v2);
    RT_FORCE_INLINE static bool LessEq3(const Vector4& v1, const Vector4& v2);
    RT_FORCE_INLINE static bool Greater3(const Vector4& v1, const Vector4& v2);
    RT_FORCE_INLINE static bool GreaterEq3(const Vector4& v1, const Vector4& v2);
    RT_FORCE_INLINE static bool NotEqual3(const Vector4& v1, const Vector4& v2);

    /**
     * Build mask of sign bits.
     */
    RT_FORCE_INLINE int GetSignMask() const;

    /**
     * For each vector component, copy value from "a" if "sel" > 0.0f, or from "b" otherwise.
     */
    RT_FORCE_INLINE static Vector4 SelectBySign(const Vector4& a, const Vector4& b, const Vector4& sel);

    /**
     * Calculate 2D dot product.
     * @return Vector4 of dot products.
     */

    RT_FORCE_INLINE static Float Dot2(const Vector4& v1, const Vector4& v2);
    /**
     * Calculate 2D dot product.
     * @return Vector4 of dot products.
     */
    RT_FORCE_INLINE static Vector4 Dot2V(const Vector4& v1, const Vector4& v2);

    /**
     * Calculate 3D dot product.
     * @return Dot product (scalar value).
     */
    RT_FORCE_INLINE static Float Dot3(const Vector4& v1, const Vector4& v2);

    /**
     * Calculate 3D dot product.
     * @return Vector4 of dot products.
     */
    RT_FORCE_INLINE static Vector4 Dot3V(const Vector4& v1, const Vector4& v2);

    /**
     * Calculate 4D dot product.
     * @return Vector4 of dot products.
     */
    RT_FORCE_INLINE static Float Dot4(const Vector4& v1, const Vector4& v2);

    /**
     * Calculate 4D dot product.
     * @return Dot product (scalar value).
     */
    RT_FORCE_INLINE static Vector4 Dot4V(const Vector4& v1, const Vector4& v2);

    /**
     * Calculate 3D cross product.
     * @return Vector4 of dot products.
     */
    RT_FORCE_INLINE static Vector4 Cross3(const Vector4& v1, const Vector4& v2);

    /**
     * Calculate length of a 2D vector.
     * @details 3rd and 4th elements are ignored.
     * @return Length of vector @p.
     */
    RT_FORCE_INLINE Float Length2() const;

    /**
     * Calculate length of a 2D vector.
     * @details 3rd and 4th elements are ignored.
     * @return Length of vector @p.
     */
    RT_FORCE_INLINE Vector4 Length2V() const;

    /**
     * Calculate length of a 3D vector.
     * @details 4th element is ignored.
     * @return Length of vector @p.
     */
    RT_FORCE_INLINE Float Length3() const;

    /**
     * Calculate length of a 3D vector.
     * @details 4th element is ignored.
     * @return Vector4 of @p v length.
     */
    RT_FORCE_INLINE Vector4 Length3V() const;

    /**
     * Calculate length of a 4D vector.
     * @return Length of vector @p.
     */
    RT_FORCE_INLINE Float Length4() const;

    /**
     * Calculate length of a 4D vector.
     * @return Length of vector @p.
     */
    RT_FORCE_INLINE Vector4 Length4V() const;

    /**
     * Normalize as 3D vector.
     * @details 4th element is ignored.
     */
    RT_FORCE_INLINE Vector4& Normalize3();
    RT_FORCE_INLINE Vector4& FastNormalize3();

    /**
     * Normalize as 4D vector.
     */
    RT_FORCE_INLINE Vector4& Normalize4();

    /**
     * Return normalized 3D vector.
     * @details 4th element is ignored.
     */
    RT_FORCE_INLINE Vector4 Normalized3() const;
    RT_FORCE_INLINE Vector4 FastNormalized3() const;

    /**
     * Return normalized 4D vector.
     */
    RT_FORCE_INLINE Vector4 Normalized4() const;

    /**
     * Reflect a 3D vector.
     * @param i incident vector
     * @param n normal vector
     * @details 4th element is ignored.
     */
    RT_FORCE_INLINE static Vector4 Reflect3(const Vector4& i, const Vector4& n);

    /**
     * Generate a plane equation from 3 points.
     * @param p1,p2,p3 Planar points
     * @return Plane equation
     */
    RT_FORCE_INLINE static Vector4 PlaneFromPoints(const Vector4& p1, const Vector4& p2, const Vector4& p3);

    /**
     * Generate a plane equation from a normal and a point.
     * @return Plane equation
     */
    RT_FORCE_INLINE static Vector4 PlaneFromNormalAndPoint(const Vector4& normal, const Vector4& p);

    /**
     * Determine plane side a point belongs to.
     * @return "true" - positive side, "false" - negative side
     */
    RT_FORCE_INLINE static bool PlanePointSide(const Vector4& plane, const Vector4& point);

    /**
     * Check if two vectors are (almost) equal.
     */
    RT_FORCE_INLINE static bool AlmostEqual(const Vector4& v1, const Vector4& v2, Float epsilon = RT_EPSILON)
    {
        Vector4 diff = Abs(v1 - v2);
        Vector4 epsilonV = Vector4(epsilon);
        return diff < epsilonV;
    }

    /**
     * Check if the vector is equal to zero Vector4()
     */
    RT_FORCE_INLINE bool IsZero() const
    {
        return _mm_movemask_ps(_mm_cmpeq_ps(v, Vector4())) == 0xF;
    }

    /**
     * Fused multiply and add.
     * @return  a * b + c
     */
    RT_FORCE_INLINE static Vector4 MulAndAdd(const Vector4& a, const Vector4& b, const Vector4& c);

    /**
     * Fused multiply and subtract.
     * @return  a * b - c
     */
    RT_FORCE_INLINE static Vector4 MulAndSub(const Vector4& a, const Vector4& b, const Vector4& c);

    /**
     * Fused multiply (negated) and add.
     * @return  - a * b + c
     */
    RT_FORCE_INLINE static Vector4 NegMulAndAdd(const Vector4& a, const Vector4& b, const Vector4& c);

    /**
     * Fused multiply (negated) and subtract.
     * @return  - a * b - c
     */
    RT_FORCE_INLINE static Vector4 NegMulAndSub(const Vector4& a, const Vector4& b, const Vector4& c);

    /**
     * Calculate horizontal minimum. Result is splatted across all elements.
     */
    RT_FORCE_INLINE Vector4 HorizontalMin() const;

    /**
     * Calculate horizontal maximum. Result is splatted across all elements.
     */
    RT_FORCE_INLINE Vector4 HorizontalMax() const;
};

// like Vector4::operator * (Float)
RT_FORCE_INLINE Vector4 operator*(Float a, const Vector4& b);


// some commonly used constants

RT_GLOBAL_CONST Vector4 VECTOR_EPSILON = { RT_EPSILON, RT_EPSILON, RT_EPSILON, RT_EPSILON };
RT_GLOBAL_CONST Vector4 VECTOR_HALVES = { 0.5f, 0.5f, 0.5f, 0.5f };
RT_GLOBAL_CONST Vector4 VECTOR_MAX = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };
RT_GLOBAL_CONST Vector4 VECTOR_ONE = { 1.0f, 1.0f, 1.0f, 1.0f };
RT_GLOBAL_CONST Vector4 VECTOR_ONE3 = { 1.0f, 1.0f, 1.0f, 0.0f };
RT_GLOBAL_CONST Vector4 VECTOR_ONE2 = { 1.0f, 1.0f, 0.0f, 0.0f };
RT_GLOBAL_CONST Vector4 VECTOR_MINUS_ONE = { -1.0f, -1.0f, -1.0f, -1.0f };

RT_GLOBAL_CONST Vector4 VECTOR_MASK_X = { 0xFFFFFFFFu, 0u, 0u, 0u };
RT_GLOBAL_CONST Vector4 VECTOR_MASK_Y = { 0u, 0xFFFFFFFFu, 0u, 0u };
RT_GLOBAL_CONST Vector4 VECTOR_MASK_Z = { 0u, 0u, 0xFFFFFFFFu, 0u };
RT_GLOBAL_CONST Vector4 VECTOR_MASK_W = { 0u, 0u, 0u, 0xFFFFFFFFu };
RT_GLOBAL_CONST Vector4 VECTOR_MASK_XY = { 0xFFFFFFFFu, 0xFFFFFFFFu, 0u, 0u };
RT_GLOBAL_CONST Vector4 VECTOR_MASK_XYZ = { 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0u };
RT_GLOBAL_CONST Vector4 VECTOR_MASK_XYZW = { 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu };

RT_GLOBAL_CONST Vector4 VECTOR_MASK_ABS = { 0x7FFFFFFFu, 0x7FFFFFFFu, 0x7FFFFFFFu, 0x7FFFFFFFu };
RT_GLOBAL_CONST Vector4 VECTOR_MASK_SIGN = { 0x80000000u, 0x80000000u, 0x80000000u, 0x80000000u };
RT_GLOBAL_CONST Vector4 VECTOR_MASK_SIGN_W = { 0u, 0u, 0u, 0x80000000u };

RT_GLOBAL_CONST Vector4 VECTOR_INV_255 = { 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f };
RT_GLOBAL_CONST Vector4 VECTOR_255 = { 255.0f, 255.0f, 255.0f, 255.0f };
RT_GLOBAL_CONST Vector4 VECTOR_X = { 1.0f, 0.0f, 0.0f, 0.0f };
RT_GLOBAL_CONST Vector4 VECTOR_Y = { 0.0f, 1.0f, 0.0f, 0.0f };
RT_GLOBAL_CONST Vector4 VECTOR_Z = { 0.0f, 0.0f, 1.0f, 0.0f };
RT_GLOBAL_CONST Vector4 VECTOR_W = { 0.0f, 0.0f, 0.0f, 1.0f };

} // namespace math
} // namespace rt

#include "Vector4Impl.h"