#pragma once

#include "../RayLib.h"
#include "../Math/Vector4.h"


namespace rt {


// Represents a ray color/weight during raytracing
// Can be RGB, CIEXYZ, wavelength powers, etc.
struct RayColor
{
    math::Vector4 values;

    RT_FORCE_INLINE RayColor() = default;
    RT_FORCE_INLINE RayColor(const math::Vector4& values) : values(values) { }
    RT_FORCE_INLINE RayColor(const RayColor& other) = default;
    RT_FORCE_INLINE RayColor& operator = (const RayColor& other) = default;

    RT_FORCE_INLINE static RayColor One()
    {
        return RayColor{ math::VECTOR_ONE };
    }

    RT_FORCE_INLINE RayColor operator + (const RayColor& other) const
    {
        return RayColor{ values + other.values };
    }

    RT_FORCE_INLINE RayColor operator * (const RayColor& other) const
    {
        return RayColor{ values * other.values };
    }

    RT_FORCE_INLINE RayColor operator * (const float factor) const
    {
        return RayColor{ values * factor };
    }

    RT_FORCE_INLINE RayColor& operator += (const RayColor& other)
    {
        values += other.values;
        return *this;
    }

    RT_FORCE_INLINE RayColor& operator *= (const RayColor& other)
    {
        values *= other.values;
        return *this;
    }

    RT_FORCE_INLINE RayColor& operator *= (const float factor)
    {
        values *= factor;
        return *this;
    }
};


} // namespace rt
