#pragma once

#include "Math.h"

namespace rt {
namespace math {


// half (16-bit) floating point type
class Half
{
public:
    RT_FORCE_INLINE Half() = default;

    RT_FORCE_INLINE Half(const Half& other) : value(other.value) { }
    RT_FORCE_INLINE explicit Half(const Uint16 other) : value(other) { }

    RT_FORCE_INLINE Half& operator = (const Half& other)
    {
        value = other.value;
        return *this;
    }

    // convert 32-bit float to half
    RT_INLINE Half(const float other);

    // convert to 32-bit float
    RT_INLINE float ToFloat() const;

private:
    union
    {
        Uint16 value;

        struct
        {
            Uint16 mantissa : 10;
            Uint16 exponent : 5;
            Uint16 sign : 1;
        } components;
    };
};


struct Half2
{
    Half x;
    Half y;
};


struct Half3
{
    Half x;
    Half y;
    Half z;
};


struct Half4
{
    Half x;
    Half y;
    Half z;
    Half w;
};


} // namespace math
} // namespace rt


#include "HalfImpl.h"
