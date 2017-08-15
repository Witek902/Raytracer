#pragma once


namespace rt {
namespace math {


// used to initialize Float constants
struct RT_ALIGN(32) Vector8f
{
    union
    {
        Float f[8];
        __m256 v;
    };

    RT_FORCE_INLINE operator Vector8() const
    {
        return v;
    }
    RT_FORCE_INLINE operator __m256() const
    {
        return v;
    }
    RT_FORCE_INLINE operator __m256i() const
    {
        return reinterpret_cast<const __m256i*>(&v)[0];
    }
    RT_FORCE_INLINE operator __m256d() const
    {
        return reinterpret_cast<const __m256d*>(&v)[0];
    }
};

// used to initialize integer constants
struct RT_ALIGN(32) Vector8i
{
    union
    {
        unsigned int u[8];
        __m256 v;
    };

    RT_FORCE_INLINE operator Vector8() const
    {
        return v;
    }
    RT_FORCE_INLINE operator __m256() const
    {
        return v;
    }
    RT_FORCE_INLINE operator __m256i() const
    {
        return reinterpret_cast<const __m256i*>(&v)[0];
    }
    RT_FORCE_INLINE operator __m256d() const
    {
        return reinterpret_cast<const __m256d*>(&v)[0];
    }
};

// some commonly used constants
const Vector8f VECTOR8_EPSILON = { { { RT_EPSILON, RT_EPSILON, RT_EPSILON, RT_EPSILON, RT_EPSILON, RT_EPSILON, RT_EPSILON, RT_EPSILON } } };
const Vector8f VECTOR8_HALVES = { { { 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f } } };
const Vector8f VECTOR8_ONE = { { { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f } } };
const Vector8f VECTOR8_MINUS_ONE = { { { -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f } } };
const Vector8i VECTOR8_MASK_X = { { { 0xFFFFFFFF, 0, 0, 0 } } };
const Vector8i VECTOR8_MASK_Y = { { { 0, 0xFFFFFFFF, 0, 0 } } };
const Vector8i VECTOR8_MASK_Z = { { { 0, 0, 0xFFFFFFFF, 0 } } };
const Vector8i VECTOR8_MASK_W = { { { 0, 0, 0, 0xFFFFFFFF } } };
const Vector8i VECTOR8_MASK_ABS = { { { 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF } } };

const Vector8f VECTOR8_INV_255 = { { { 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f } } };
const Vector8f VECTOR8_255 = { { { 255.0f, 255.0f, 255.0f, 255.0f, 255.0f, 255.0f, 255.0f, 255.0f } } };


} // namespace math
} // namespace rt
