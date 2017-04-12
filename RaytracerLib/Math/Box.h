#pragma once

#include "Math.h"
#include "Vector.h"
#include "Matrix.h"

namespace rt {
namespace math {

/**
 * Axis Aligned Box
 */
class RT_ALIGN(16) Box
{
public:
    Vector min;
    Vector max;

    RT_FORCE_INLINE Box() : min(), max() {}
    RT_FORCE_INLINE Box(const Vector& min, const Vector& max) : min(min), max(max) {}

    // create box from center point and radius (e.g. bounding box of a sphere)
    RT_FORCE_INLINE Box(const Vector& center, float radius)
    {
        min = center - Vector::Splat(radius);
        max = center + Vector::Splat(radius);
    }

    // merge boxes
    RT_FORCE_INLINE Box(const Box& a, const Box& b)
    {
        min = Vector::Min(a.min, b.min);
        max = Vector::Max(a.max, b.max);
    }

    RT_FORCE_INLINE Vector GetCenter() const;
    RT_FORCE_INLINE Vector GetVertex(int id) const;
    RT_FORCE_INLINE Vector SupportVertex(const Vector& dir) const;
    RT_FORCE_INLINE void MakeFromPoints(const Vector* pPoints, int number);
    RT_FORCE_INLINE float SurfaceArea() const;
    RT_FORCE_INLINE float Volume() const;
};


Vector Box::GetCenter() const
{
    return (min + max) * 0.5f;
}

Vector Box::SupportVertex(const Vector& dir) const
{
    return Vector::SelectBySign(max, min, dir);
}

float Box::SurfaceArea() const
{
    Vector size = max - min;
    return size.f[0] * (size.f[1] + size.f[2]) + size.f[1] * size.f[2];
}

float Box::Volume() const
{
    Vector size = max - min;
    return size.f[0] * size.f[1] * size.f[2];
}


} // namespace math
} // namespace rt