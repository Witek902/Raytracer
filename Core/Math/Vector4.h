#pragma once

#include "Math.h"

#include "Float2.h"
#include "Float3.h"
#include "VectorBool4.h"

namespace rt {
namespace math {

/**
 * 4-element SIMD vector
 */
struct RT_ALIGN(16) Vector4
{
    union
    {
        float f[4];
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

    // constructors
    RT_FORCE_INLINE Vector4();
    RT_FORCE_INLINE Vector4(const Vector4& other);
    RT_FORCE_INLINE Vector4(const __m128& src);
    RT_FORCE_INLINE static const Vector4 Zero();
    RT_FORCE_INLINE explicit Vector4(const float s); // splat
    RT_FORCE_INLINE Vector4(const float x, const float y, const float z, const float w);
    RT_FORCE_INLINE Vector4(const Int32 x, const Int32 y, const Int32 z, const Int32 w);
    RT_FORCE_INLINE Vector4(const Uint32 x, const Uint32 y, const Uint32 z, const Uint32 w);
    RT_FORCE_INLINE explicit Vector4(const float* src);
    RT_FORCE_INLINE explicit Vector4(const Float2& src);
    RT_FORCE_INLINE explicit Vector4(const Float3& src);
    RT_FORCE_INLINE Vector4& operator = (const Vector4& other);

    RT_FORCE_INLINE static const Vector4 FromInteger(Int32 x);
    RT_FORCE_INLINE static const Vector4 FromIntegers(Int32 x, Int32 y, Int32 z, Int32 w);
    RT_FORCE_INLINE static const Vector4 FromHalves(const Half* src);

    RT_FORCE_INLINE explicit operator float() const { return x; }
    RT_FORCE_INLINE operator __m128() const { return v; }
    RT_FORCE_INLINE operator __m128i() const { return reinterpret_cast<const __m128i*>(&v)[0]; }
    RT_FORCE_INLINE float operator[] (Uint32 index) const { return f[index]; }

    // simple arithmetics
    RT_FORCE_INLINE const Vector4 operator- () const;
    RT_FORCE_INLINE const Vector4 operator+ (const Vector4& b) const;
    RT_FORCE_INLINE const Vector4 operator- (const Vector4& b) const;
    RT_FORCE_INLINE const Vector4 operator* (const Vector4& b) const;
    RT_FORCE_INLINE const Vector4 operator/ (const Vector4& b) const;
    RT_FORCE_INLINE const Vector4 operator* (float b) const;
    RT_FORCE_INLINE const Vector4 operator/ (float b) const;
    RT_FORCE_INLINE Vector4& operator+= (const Vector4& b);
    RT_FORCE_INLINE Vector4& operator-= (const Vector4& b);
    RT_FORCE_INLINE Vector4& operator*= (const Vector4& b);
    RT_FORCE_INLINE Vector4& operator/= (const Vector4& b);
    RT_FORCE_INLINE Vector4& operator*= (float b);
    RT_FORCE_INLINE Vector4& operator/= (float b);

    RT_FORCE_INLINE const VectorBool4 operator == (const Vector4& b) const;
    RT_FORCE_INLINE const VectorBool4 operator < (const Vector4& b) const;
    RT_FORCE_INLINE const VectorBool4 operator <= (const Vector4& b) const;
    RT_FORCE_INLINE const VectorBool4 operator > (const Vector4& b) const;
    RT_FORCE_INLINE const VectorBool4 operator >= (const Vector4& b) const;
    RT_FORCE_INLINE const VectorBool4 operator != (const Vector4& b) const;

    // bitwise logic operations
    RT_FORCE_INLINE const Vector4 operator& (const Vector4& b) const;
    RT_FORCE_INLINE const Vector4 operator| (const Vector4& b) const;
    RT_FORCE_INLINE const Vector4 operator^ (const Vector4& b) const;
    RT_FORCE_INLINE Vector4& operator&= (const Vector4& b);
    RT_FORCE_INLINE Vector4& operator|= (const Vector4& b);
    RT_FORCE_INLINE Vector4& operator^= (const Vector4& b);

    RT_FORCE_INLINE const Vector4 SplatX() const;
    RT_FORCE_INLINE const Vector4 SplatY() const;
    RT_FORCE_INLINE const Vector4 SplatZ() const;
    RT_FORCE_INLINE const Vector4 SplatW() const;

    // Change sign of selected elements
    template<Uint32 flipX, Uint32 flipY, Uint32 flipZ, Uint32 flipW>
    RT_FORCE_INLINE const Vector4 ChangeSign() const;

    // Prepare mask vector
    template<Uint32 maskX, Uint32 maskY, Uint32 maskZ, Uint32 maskW>
    RT_FORCE_INLINE static const Vector4 MakeMask();

    // Rearrange vector elements
    template<Uint32 ix = 0, Uint32 iy = 1, Uint32 iz = 2, Uint32 iw = 3>
    RT_FORCE_INLINE const Vector4 Swizzle() const;

    // Convert 4 uint8 to a Vector4
    RT_FORCE_INLINE static const Vector4 Load4(const Uint8* src);

    // Convert 3 uint8 to a Vector4 and scale to 0...1 range
    RT_FORCE_INLINE static const Vector4 LoadBGR_UNorm(const Uint8* src);

    // Convert to 3 uint8 values
    RT_FORCE_INLINE void StoreBGR_NonTemporal(Uint8* dest) const;

    // Convert a Vector4 to 4 unsigned chars
    RT_FORCE_INLINE void Store4_NonTemporal(Uint8* dest) const;

    RT_FORCE_INLINE void Store(float* dest) const;
    RT_FORCE_INLINE void Store(Float2* dest) const;
    RT_FORCE_INLINE void Store(Float3* dest) const;
    RT_FORCE_INLINE Float2 ToFloat2() const;
    RT_FORCE_INLINE Float3 ToFloat3() const;

    RT_FORCE_INLINE static const Vector4 Floor(const Vector4& v);
    RT_FORCE_INLINE static const Vector4 Sqrt4(const Vector4& v);
    RT_FORCE_INLINE static const Vector4 Reciprocal(const Vector4& v);
    RT_FORCE_INLINE static const Vector4 FastReciprocal(const Vector4& v);
    RT_FORCE_INLINE static const Vector4 Lerp(const Vector4& v1, const Vector4& v2, const Vector4& weight);
    RT_FORCE_INLINE static const Vector4 Lerp(const Vector4& v1, const Vector4& v2, float weight);
    RT_FORCE_INLINE static const Vector4 Min(const Vector4& a, const Vector4& b);
    RT_FORCE_INLINE static const Vector4 Max(const Vector4& a, const Vector4& b);
    RT_FORCE_INLINE static const Vector4 Abs(const Vector4& v);
    RT_FORCE_INLINE const Vector4 Clamped(const Vector4& min, const Vector4& max) const;

    // Build mask of sign bits
    RT_FORCE_INLINE int GetSignMask() const;

    // For each vector component, copy value from "a" if "sel" is "false", or from "b" otherwise
    RT_FORCE_INLINE static const Vector4 Select(const Vector4& a, const Vector4& b, const VectorBool4& sel);

    // Calculate 2D dot product (scalar result)
    RT_FORCE_INLINE static float Dot2(const Vector4& v1, const Vector4& v2);

    // Calculate 2D dot product (vector result)
    RT_FORCE_INLINE static const Vector4 Dot2V(const Vector4& v1, const Vector4& v2);

    // Calculate 3D dot product (scalar result)
    RT_FORCE_INLINE static float Dot3(const Vector4& v1, const Vector4& v2);

    // Calculate 3D dot product (vector result)
    RT_FORCE_INLINE static const Vector4 Dot3V(const Vector4& v1, const Vector4& v2);

    // Calculate 4D dot product (scalar result)
    RT_FORCE_INLINE static float Dot4(const Vector4& v1, const Vector4& v2);

    // Calculate 4D dot product (vector result)
    RT_FORCE_INLINE static const Vector4 Dot4V(const Vector4& v1, const Vector4& v2);

    // Calculate 3D cross product
    RT_FORCE_INLINE static const Vector4 Cross3(const Vector4& v1, const Vector4& v2);

    // Square length of a 2D vector (scalar result)
    RT_FORCE_INLINE float SqrLength2() const;

    // Length of a 2D vector (scalar result)
    RT_FORCE_INLINE float Length2() const;

    // Length of a 2D vector (vector result)
    RT_FORCE_INLINE const Vector4 Length2V() const;

    // Length of a 3D vector (scalar result)
    RT_FORCE_INLINE float Length3() const;

    // Square length of a 3D vector (scalar result)
    RT_FORCE_INLINE float SqrLength3() const;

    // Length of a 3D vector (vector result)
    RT_FORCE_INLINE const Vector4 Length3V() const;

    // Length of a 4D vector (scalar result)
    RT_FORCE_INLINE float Length4() const;

    // Length of a 4D vector (vector result)
    RT_FORCE_INLINE const Vector4 Length4V() const;

    // Square length of a 4D vector (scalar result)
    RT_FORCE_INLINE float SqrLength4() const;

    // Normalize as 3D vector
    RT_FORCE_INLINE Vector4& Normalize3();
    RT_FORCE_INLINE Vector4& FastNormalize3();

    // Normalize as 4D vector
    RT_FORCE_INLINE Vector4& Normalize4();

    // Return normalized 3D vector
    RT_FORCE_INLINE const Vector4 Normalized3() const;
    RT_FORCE_INLINE const Vector4 FastNormalized3() const;

    // Return normalized 4D vector
    RT_FORCE_INLINE const Vector4 Normalized4() const;

    // Reflect a 3D vector
    RT_FORCE_INLINE static const Vector4 Reflect3(const Vector4& i, const Vector4& n);

    // Refract a 3D vector
    static const Vector4 Refract3(const Vector4& i, const Vector4& n, float eta);

    // Check if two vectors are (almost) equal
    RT_FORCE_INLINE static bool AlmostEqual(const Vector4& v1, const Vector4& v2, float epsilon = RT_EPSILON);

    // Check if the vector is equal to zero
    RT_FORCE_INLINE const VectorBool4 IsZero() const;

    // Check if any component is NaN
    RT_FORCE_INLINE const VectorBool4 IsNaN() const;

    // Check if any component is an infinity
    RT_FORCE_INLINE const VectorBool4 IsInfinite() const;

    // Check if is not NaN or infinity
    RT_FORCE_INLINE bool IsValid() const;

    // Fused multiply and add (a * b + c)
    RT_FORCE_INLINE static const Vector4 MulAndAdd(const Vector4& a, const Vector4& b, const Vector4& c);
    RT_FORCE_INLINE static const Vector4 MulAndAdd(const Vector4& a, const float b, const Vector4& c);

    // Fused multiply and subtract (a * b - c)
    RT_FORCE_INLINE static const Vector4 MulAndSub(const Vector4& a, const Vector4& b, const Vector4& c);
    RT_FORCE_INLINE static const Vector4 MulAndSub(const Vector4& a, const float b, const Vector4& c);

    // Fused multiply (negated) and add (-a * b + c)
    RT_FORCE_INLINE static const Vector4 NegMulAndAdd(const Vector4& a, const Vector4& b, const Vector4& c);
    RT_FORCE_INLINE static const Vector4 NegMulAndAdd(const Vector4& a, const float b, const Vector4& c);

    // Fused multiply (negated) and subtract (-a * b - c)
    RT_FORCE_INLINE static const Vector4 NegMulAndSub(const Vector4& a, const Vector4& b, const Vector4& c);
    RT_FORCE_INLINE static const Vector4 NegMulAndSub(const Vector4& a, const float b, const Vector4& c);

    // Calculate horizontal maximum. Result is splatted across all elements
    RT_FORCE_INLINE const Vector4 HorizontalMax() const;

    // transpose 3x3 matrix
    RT_FORCE_INLINE static void Transpose3(Vector4& a, Vector4& b, Vector4& c);

    // make vector "v" orthogonal to "reference" vector
    RT_FORCE_INLINE static const Vector4 Orthogonalize(const Vector4& v, const Vector4& reference);
};

// like Vector4::operator * (float)
RT_FORCE_INLINE const Vector4 operator*(float a, const Vector4& b);


// some commonly used constants

RT_GLOBAL_CONST Vector4 VECTOR_EPSILON = { RT_EPSILON, RT_EPSILON, RT_EPSILON, RT_EPSILON };
RT_GLOBAL_CONST Vector4 VECTOR_HALVES = { 0.5f, 0.5f, 0.5f, 0.5f };
RT_GLOBAL_CONST Vector4 VECTOR_MIN = { std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min() };
RT_GLOBAL_CONST Vector4 VECTOR_MAX = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
RT_GLOBAL_CONST Vector4 VECTOR_INF = { std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity() };
RT_GLOBAL_CONST Vector4 VECTOR_ONE = { 1.0f, 1.0f, 1.0f, 1.0f };
RT_GLOBAL_CONST Vector4 VECTOR_ONE3 = { 1.0f, 1.0f, 1.0f, 0.0f };
RT_GLOBAL_CONST Vector4 VECTOR_ONE2 = { 1.0f, 1.0f, 0.0f, 0.0f };
RT_GLOBAL_CONST Vector4 VECTOR_MINUS_ONE = { -1.0f, -1.0f, -1.0f, -1.0f };

RT_GLOBAL_CONST Vector4 VECTOR_MASK_ABS = { 0x7FFFFFFFu, 0x7FFFFFFFu, 0x7FFFFFFFu, 0x7FFFFFFFu };
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