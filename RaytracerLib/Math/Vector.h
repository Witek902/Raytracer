#pragma once

#include "Math.h"

#define RT_USE_FMA

namespace rt {
namespace math {

/**
 * 4-element SIMD vector.
 */
struct RT_ALIGN(16) Vector
{
    union
    {
        Float f[4];
        int i[4];
        unsigned int u[4];
        __m128 v;
    };

    // conversion to/from SSE types
    RT_FORCE_INLINE operator __m128() const
    {
        return v;
    }
    RT_FORCE_INLINE operator __m128i() const
    {
        return reinterpret_cast<const __m128i*>(&v)[0];
    }
    RT_FORCE_INLINE operator __m128d() const
    {
        return reinterpret_cast<const __m128d*>(&v)[0];
    }
    RT_FORCE_INLINE Vector(const __m128& src)
    {
        v = src;
    }

    // constructors
    RT_FORCE_INLINE Vector();
    RT_FORCE_INLINE explicit Vector(Float x, Float y = 0.0f, Float z = 0.0f, Float w = 0.0f);
    RT_FORCE_INLINE explicit Vector(int x, int y = 0, int z = 0, int w = 0);
    RT_FORCE_INLINE Vector(const Float* src);
    RT_FORCE_INLINE void Set(Float scalar);

    // element access
    RT_FORCE_INLINE Float operator[] (int index) const
    {
        return f[index];
    }

    // element access (reference)
    RT_FORCE_INLINE Float& operator[] (int index)
    {
        return f[index];
    }

    /// simple arithmetics
    RT_FORCE_INLINE Vector operator- () const;
    RT_FORCE_INLINE Vector operator+ (const Vector& b) const;
    RT_FORCE_INLINE Vector operator- (const Vector& b) const;
    RT_FORCE_INLINE Vector operator* (const Vector& b) const;
    RT_FORCE_INLINE Vector operator/ (const Vector& b) const;
    RT_FORCE_INLINE Vector operator* (Float b) const;
    RT_FORCE_INLINE Vector operator/ (Float b) const;
    RT_FORCE_INLINE Vector& operator+= (const Vector& b);
    RT_FORCE_INLINE Vector& operator-= (const Vector& b);
    RT_FORCE_INLINE Vector& operator*= (const Vector& b);
    RT_FORCE_INLINE Vector& operator/= (const Vector& b);
    RT_FORCE_INLINE Vector& operator*= (Float b);
    RT_FORCE_INLINE Vector& operator/= (Float b);

    /// comparison operators (returns true, if all the elements satisfy the equation)
    RT_FORCE_INLINE bool operator== (const Vector& b) const;
    RT_FORCE_INLINE bool operator< (const Vector& b) const;
    RT_FORCE_INLINE bool operator<= (const Vector& b) const;
    RT_FORCE_INLINE bool operator> (const Vector& b) const;
    RT_FORCE_INLINE bool operator>= (const Vector& b) const;
    RT_FORCE_INLINE bool operator!= (const Vector& b) const;

    /// bitwise logic operations
    RT_FORCE_INLINE Vector operator& (const Vector& b) const;
    RT_FORCE_INLINE Vector operator| (const Vector& b) const;
    RT_FORCE_INLINE Vector operator^ (const Vector& b) const;
    RT_FORCE_INLINE Vector& operator&= (const Vector& b);
    RT_FORCE_INLINE Vector& operator|= (const Vector& b);
    RT_FORCE_INLINE Vector& operator^= (const Vector& b);

    RT_FORCE_INLINE Vector SplatX() const;
    RT_FORCE_INLINE Vector SplatY() const;
    RT_FORCE_INLINE Vector SplatZ() const;
    RT_FORCE_INLINE Vector SplatW() const;

    /**
     * Rearrange vector elements.
     */
    template<bool x = false, bool y = false, bool z = false, bool w = false>
    RT_FORCE_INLINE Vector ChangeSign() const;

    /**
     * Rearrange vector elements.
     */
    template<Uint32 ix = 0, Uint32 iy = 1, Uint32 iz = 2, Uint32 iw = 3>
    RT_FORCE_INLINE Vector Swizzle() const;

    /**
     * Convert 4 uint8 to a Vector.
     */
    RT_FORCE_INLINE static Vector Load4(const Uint8* src);

    /**
     * Convert a Vector to 4 unsigned chars.
     */
    RT_FORCE_INLINE void Store4(Uint8* dest) const;

    RT_FORCE_INLINE void Store(Float* dest) const;
    RT_FORCE_INLINE static Vector Splat(Float f);

    RT_FORCE_INLINE static Vector Floor(const Vector& v);
    RT_FORCE_INLINE static Vector Sqrt(const Vector& v);
    RT_FORCE_INLINE static Vector Sqrt4(const Vector& v);
    RT_FORCE_INLINE static Vector Reciprocal(const Vector& v);
    RT_FORCE_INLINE static Vector FastReciprocal(const Vector& v);
    RT_FORCE_INLINE static Vector Lerp(const Vector& v1, const Vector& v2, const Vector& weight);
    RT_FORCE_INLINE static Vector Lerp(const Vector& v1, const Vector& v2, Float weight);
    RT_FORCE_INLINE static Vector Min(const Vector& a, const Vector& b);
    RT_FORCE_INLINE static Vector Max(const Vector& a, const Vector& b);
    RT_FORCE_INLINE static Vector Abs(const Vector& v);
    RT_FORCE_INLINE Vector Clamped(const Vector& min, const Vector& max) const;

    RT_FORCE_INLINE static int EqualMask(const Vector& v1, const Vector& v2);
    RT_FORCE_INLINE static int LessMask(const Vector& v1, const Vector& v2);
    RT_FORCE_INLINE static int LessEqMask(const Vector& v1, const Vector& v2);
    RT_FORCE_INLINE static int GreaterMask(const Vector& v1, const Vector& v2);
    RT_FORCE_INLINE static int GreaterEqMask(const Vector& v1, const Vector& v2);
    RT_FORCE_INLINE static int NotEqualMask(const Vector& v1, const Vector& v2);

    RT_FORCE_INLINE static bool Equal2(const Vector& v1, const Vector& v2);
    RT_FORCE_INLINE static bool Less2(const Vector& v1, const Vector& v2);
    RT_FORCE_INLINE static bool LessEq2(const Vector& v1, const Vector& v2);
    RT_FORCE_INLINE static bool Greater2(const Vector& v1, const Vector& v2);
    RT_FORCE_INLINE static bool GreaterEq2(const Vector& v1, const Vector& v2);
    RT_FORCE_INLINE static bool NotEqual2(const Vector& v1, const Vector& v2);

    RT_FORCE_INLINE static bool Equal3(const Vector& v1, const Vector& v2);
    RT_FORCE_INLINE static bool Less3(const Vector& v1, const Vector& v2);
    RT_FORCE_INLINE static bool LessEq3(const Vector& v1, const Vector& v2);
    RT_FORCE_INLINE static bool Greater3(const Vector& v1, const Vector& v2);
    RT_FORCE_INLINE static bool GreaterEq3(const Vector& v1, const Vector& v2);
    RT_FORCE_INLINE static bool NotEqual3(const Vector& v1, const Vector& v2);

    /**
     * For each vector component, copy value from "a" if "sel" > 0.0f, or from "b" otherwise.
     */
    RT_FORCE_INLINE static Vector SelectBySign(const Vector& a, const Vector& b, const Vector& sel);

    /**
     * Calculate 3D dot product.
     * @return Dot product (scalar value).
     */
    RT_FORCE_INLINE static Float Dot3(const Vector& v1, const Vector& v2);

    /**
     * Calculate 3D dot product.
     * @return Vector of dot products.
     */
    RT_FORCE_INLINE static Vector Dot3V(const Vector& v1, const Vector& v2);

    /**
     * Calculate 4D dot product.
     * @return Vector of dot products.
     */
    RT_FORCE_INLINE static Float Dot4(const Vector& v1, const Vector& v2);

    /**
     * Calculate 4D dot product.
     * @return Dot product (scalar value).
     */
    RT_FORCE_INLINE static Vector Dot4V(const Vector& v1, const Vector& v2);

    /**
     * Calculate 3D cross product.
     * @return Vector of dot products.
     */
    RT_FORCE_INLINE static Vector Cross3(const Vector& v1, const Vector& v2);

    /**
     * Calculate length of a 3D vector.
     * @details 4th element is ignored.
     * @return Length of vector @p.
     */
    RT_FORCE_INLINE Float Length3() const;

    /**
     * Calculate length of a 3D vector.
     * @details 4th element is ignored.
     * @return Vector of @p v length.
     */
    RT_FORCE_INLINE Vector Length3V() const;

    /**
     * Calculate length of a 4D vector.
     * @return Length of vector @p.
     */
    RT_FORCE_INLINE Float Length4() const;

    /**
     * Calculate length of a 4D vector.
     * @return Length of vector @p.
     */
    RT_FORCE_INLINE Vector Length4V() const;

    /**
     * Normalize as 3D vector.
     * @details 4th element is ignored.
     */
    RT_FORCE_INLINE Vector& Normalize3();
    RT_FORCE_INLINE Vector& FastNormalize3();

    /**
     * Normalize as 4D vector.
     */
    RT_FORCE_INLINE Vector& Normalize4();

    /**
     * Return normalized 3D vector.
     * @details 4th element is ignored.
     */
    RT_FORCE_INLINE Vector Normalized3() const;
    RT_FORCE_INLINE Vector FastNormalized3() const;

    /**
     * Return normalized 4D vector.
     */
    RT_FORCE_INLINE Vector Normalized4() const;

    /**
     * Reflect a 3D vector.
     * @param i incident vector
     * @param n normal vector
     * @details 4th element is ignored.
     */
    RT_FORCE_INLINE static Vector Reflect3(const Vector& i, const Vector& n);

    /**
     * Generate a plane equation from 3 points.
     * @param p1,p2,p3 Planar points
     * @return Plane equation
     */
    RT_FORCE_INLINE static Vector PlaneFromPoints(const Vector& p1, const Vector& p2, const Vector& p3);

    /**
     * Generate a plane equation from a normal and a point.
     * @return Plane equation
     */
    RT_FORCE_INLINE static Vector PlaneFromNormalAndPoint(const Vector& normal, const Vector& p);

    /**
     * Determine plane side a point belongs to.
     * @return "true" - positive side, "false" - negative side
     */
    RT_FORCE_INLINE static bool PlanePointSide(const Vector& plane, const Vector& point);

    /**
     * Check if two vectors are (almost) equal.
     */
    RT_FORCE_INLINE static bool AlmostEqual(const Vector& v1, const Vector& v2, Float epsilon = RT_EPSILON)
    {
        Vector diff = Abs(v1 - v2);
        Vector epsilonV = Vector::Splat(epsilon);
        return diff < epsilonV;
    }

    /**
     * Fused multiply and add.
     * @return  a * b + c
     */
    RT_FORCE_INLINE static Vector MulAndAdd(const Vector& a, const Vector& b, const Vector& c);

    /**
     * Fused multiply and subtract.
     * @return  a * b - c
     */
    RT_FORCE_INLINE static Vector MulAndSub(const Vector& a, const Vector& b, const Vector& c);
        
    /**
     * Fused multiply (negated) and add.
     * @return  - a * b + c
     */
    RT_FORCE_INLINE static Vector NegMulAndAdd(const Vector& a, const Vector& b, const Vector& c);

    /**
     * Fused multiply (negated) and subtract.
     * @return  - a * b - c
     */
    RT_FORCE_INLINE static Vector NegMulAndSub(const Vector& a, const Vector& b, const Vector& c);
};

// like Vector::operator * (Float)
RT_FORCE_INLINE Vector operator*(Float a, const Vector& b);


} // namespace math
} // namespace rt

#include "VectorConstants.h"
#include "VectorImpl.h"