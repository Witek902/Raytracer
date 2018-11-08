#pragma once

#include "Math.h"

namespace rt {
namespace math {

// 2D rectangle
template<typename T>
class Rectangle
{
public:
    RT_FORCE_INLINE constexpr Rectangle()
        : minX(T(0))
        , maxX(T(0))
        , minY(T(0))
        , maxY(T(0))
    { }

    RT_FORCE_INLINE constexpr Rectangle(T minX, T maxX, T minY, T maxY)
        : minX(minX)
        , maxX(maxX)
        , minY(minY)
        , maxY(maxY)
    { }

    RT_FORCE_INLINE constexpr T Width() const
    {
        return maxX - minX;
    }

    RT_FORCE_INLINE constexpr T Height() const
    {
        return maxY - minY;
    }

    T minX;
    T maxX;
    T minY;
    T maxY;
};


} // namespace math
} // namespace rt
