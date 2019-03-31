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

    RT_FORCE_INLINE Box() = default;

    RT_FORCE_INLINE explicit Box(const Vector4& point)
        : min(point)
        , max(point)
    {}

    RT_FORCE_INLINE Box(const Vector4& min, const Vector4& max)
        : min(min)
        , max(max)
    {}

    RT_FORCE_INLINE Box(const Vector4& a, const Vector4& b, const Vector4& c)
        : min(Vector4::Min(a, Vector4::Min(b, c)))
        , max(Vector4::Max(a, Vector4::Max(b, c)))
    {}

    RT_FORCE_INLINE static const Box Empty()
    {
        return { VECTOR_MAX, -VECTOR_MAX };
    }

    RT_FORCE_INLINE static const Box Full()
    {
        return { -VECTOR_MAX, VECTOR_MAX };
    }

    // create box from center point and radius (e.g. bounding box of a sphere)
    RT_FORCE_INLINE Box(const Vector4& center, float radius)
        : min(center - Vector4(radius))
        , max(center + Vector4(radius))
    {}

    // merge boxes
    RT_FORCE_INLINE Box(const Box& a, const Box& b)
        : min(Vector4::Min(a.min, b.min))
        , max(Vector4::Max(a.max, b.max))
    {}

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
        return size.x * (size.y + size.z) + size.y * size.z;
    }

    RT_FORCE_INLINE float Volume() const
    {
        Vector4 size = max - min;
        return size.x * size.y * size.z;
    }

    RT_FORCE_INLINE Box& AddPoint(const Vector4& point)
    {
        min = Vector4::Min(min, point);
        max = Vector4::Max(max, point);
        return *this;
    }
};


} // namespace math
} // namespace rt