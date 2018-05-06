#pragma once


namespace rt {
namespace math {


// used to initialize Float constants
struct RT_ALIGN(16) Vectorf
{
    union
    {
        Float f[4];
        __m128 v;
    };

    RT_FORCE_INLINE operator Vector4() const
    {
        return v;
    }
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
};

// used to initialize integer constants
struct RT_ALIGN(16) Vectori
{
    union
    {
        unsigned int u[4];
        __m128 v;
    };

    RT_FORCE_INLINE operator Vector4() const
    {
        return v;
    }
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
};

// some commonly used constants
const Vectorf VECTOR_EPSILON = { { { RT_EPSILON, RT_EPSILON, RT_EPSILON, RT_EPSILON } } };
const Vectorf VECTOR_HALVES = { { { 0.5f, 0.5f, 0.5f, 0.5f } } };
const Vectorf VECTOR_MAX = { { { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX } } };
const Vectorf VECTOR_ONE = { { { 1.0f, 1.0f, 1.0f, 1.0f } } };
const Vectorf VECTOR_ONE3 = { { { 1.0f, 1.0f, 1.0f, 0.0f } } };
const Vectorf VECTOR_ONE2 = { { { 1.0f, 1.0f, 0.0f, 0.0f } } };
const Vectorf VECTOR_MINUS_ONE = { { { -1.0f, -1.0f, -1.0f, -1.0f } } };
const Vectori VECTOR_MASK_X = { { { 0xFFFFFFFF, 0, 0, 0 } } };
const Vectori VECTOR_MASK_Y = { { { 0, 0xFFFFFFFF, 0, 0 } } };
const Vectori VECTOR_MASK_Z = { { { 0, 0, 0xFFFFFFFF, 0 } } };
const Vectori VECTOR_MASK_W = { { { 0, 0, 0, 0xFFFFFFFF } } };
const Vectori VECTOR_MASK_XY = { { { 0xFFFFFFFF, 0xFFFFFFFF, 0, 0 } } };
const Vectori VECTOR_MASK_XYZ = { { { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0 } } };
const Vectori VECTOR_MASK_XYZW = { { { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF } } };
const Vectori VECTOR_MASK_ABS = { { { 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF } } };
const Vectori VECTOR_MASK_SIGN = { { { 0x80000000, 0x80000000, 0x80000000, 0x80000000 } } };

const Vectorf VECTOR_INV_255 = { { { 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f } } };
const Vectorf VECTOR_255 = { { { 255.0f, 255.0f, 255.0f, 255.0f } } };

// identity matrix rows
const Vectorf VECTOR_X = { { { 1.0f, 0.0f, 0.0f, 0.0f } } };
const Vectorf VECTOR_Y = { { { 0.0f, 1.0f, 0.0f, 0.0f } } };
const Vectorf VECTOR_Z = { { { 0.0f, 0.0f, 1.0f, 0.0f } } };
const Vectorf VECTOR_W = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };


} // namespace math
} // namespace rt
