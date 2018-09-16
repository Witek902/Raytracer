#pragma once

#include "Math.h"
#include "Vector4.h"

namespace rt {
namespace math {

/**
 * Axis Aligned Box
 */
class RT_ALIGN(16) Box
{
public:
    Vector4 min;
    Vector4 max;

    RT_FORCE_INLINE Box() : min(), max() {}
    RT_FORCE_INLINE Box(const Vector4& min, const Vector4& max) : min(min), max(max) {}

    Box(const Vector4& a, const Vector4& b, const Vector4& c);

    RT_FORCE_INLINE static const Box Empty()
    {
        return Box(Vector4(VECTOR_MAX), -Vector4(VECTOR_MAX));
    }

    // create box from center point and radius (e.g. bounding box of a sphere)
    RT_FORCE_INLINE Box(const Vector4& center, float radius)
    {
        min = center - Vector4(radius);
        max = center + Vector4(radius);
    }

    // merge boxes
    RT_FORCE_INLINE Box(const Box& a, const Box& b)
    {
        min = Vector4::Min(a.min, b.min);
        max = Vector4::Max(a.max, b.max);
    }

    RT_FORCE_INLINE const Box operator + (const Vector4& offset) const
    {
        return Box{ min + offset, max + offset };
    }

    RT_FORCE_INLINE const Vector4 GetCenter() const
    {
        return (min + max) * 0.5f;
    }

    RT_FORCE_INLINE float SurfaceArea() const
    {
        Vector4 size = max - min;
        return size.f[0] * (size.f[1] + size.f[2]) + size.f[1] * size.f[2];
    }

    RT_FORCE_INLINE float Volume() const
    {
        Vector4 size = max - min;
        return size.f[0] * size.f[1] * size.f[2];
    }
};


} // namespace math
} // namespace r