#pragma once

#include "Wavelength.h"

namespace rt {

struct Spectrum;

// Represents a ray color/weight during raytracing
// The color values corresponds to wavelength values.
struct RayColor
{
    Wavelength::ValueType value;

    RT_FORCE_INLINE RayColor() = default;
    RT_FORCE_INLINE RayColor(const RayColor& other) = default;
    RT_FORCE_INLINE RayColor& operator = (const RayColor& other) = default;

    RT_FORCE_INLINE explicit RayColor(const float val) : value(val) { }
    RT_FORCE_INLINE explicit RayColor(const Wavelength::ValueType& val) : value(val) { }

    RT_FORCE_INLINE static const RayColor Zero()
    {
        return RayColor{ Wavelength::ValueType::Zero() };
    }

    RT_FORCE_INLINE static const RayColor One()
    {
#ifdef RT_ENABLE_SPECTRAL_RENDERING
        return RayColor{ math::VECTOR8_ONE };
#else
        return RayColor{ math::VECTOR_ONE };
#endif
    }

    RT_FORCE_INLINE static const RayColor SingleWavelengthFallback()
    {
#ifdef RT_ENABLE_SPECTRAL_RENDERING
        return RayColor{ Wavelength::ValueType((float)Wavelength::NumComponents, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f) };
#else
        return RayColor{ math::VECTOR_ONE };
#endif
    }

    RT_FORCE_INLINE const RayColor operator + (const RayColor& other) const
    {
        return RayColor{ value + other.value };
    }

    RT_FORCE_INLINE const RayColor operator * (const RayColor& other) const
    {
        return RayColor{ value * other.value };
    }

    RT_FORCE_INLINE const RayColor operator / (const RayColor& other) const
    {
        return RayColor{ value / other.value };
    }

    RT_FORCE_INLINE const RayColor operator * (const float factor) const
    {
        return RayColor{ value * factor };
    }

    RT_FORCE_INLINE const RayColor operator / (const float factor) const
    {
        return RayColor{ value / factor };
    }

    RT_FORCE_INLINE RayColor& operator += (const RayColor& other)
    {
        value += other.value;
        return *this;
    }

    RT_FORCE_INLINE RayColor& operator *= (const RayColor& other)
    {
        value *= other.value;
        return *this;
    }

    RT_FORCE_INLINE RayColor& operator *= (const float factor)
    {
        value *= factor;
        return *this;
    }

    RT_FORCE_INLINE bool AlmostZero() const
    {
        return Wavelength::ValueType::AlmostEqual(value, Wavelength::ValueType::Zero());
    }

    RT_FORCE_INLINE float Max() const
    {
        Wavelength::ValueType maskedValue = value;
#ifndef RT_ENABLE_SPECTRAL_RENDERING
        maskedValue &= math::Vector4::MakeMask<1,1,1,0>();
#endif
        return maskedValue.HorizontalMax()[0];
    }

    RT_FORCE_INLINE bool IsValid() const
    {
#ifndef RT_ENABLE_SPECTRAL_RENDERING
        // exception: in spectral rendering these values can get below zero due to RGB->Spectrum conversion
        if (!(value >= Wavelength::ValueType::Zero()).All())
        {
            return false;
        }
#endif
        return value.IsValid();
    }

    RT_FORCE_INLINE static const RayColor Lerp(const RayColor& a, const RayColor& b, const float factor)
    {
        return RayColor{ Wavelength::ValueType::Lerp(a.value, b.value, factor) };
    }

    // calculate ray color values for given wavelength and linear RGB values
    static const RayColor BlackBody(const Wavelength& wavelength, const float temperature);

    // calculate ray color values for given wavelength and spectrum
    static const RayColor Resolve(const Wavelength& wavelength, const Spectrum& spectrum);

    // convert to CIE XYZ / RGB tristimulus values
    // NOTE: when spectral rendering is disabled, this function does nothing (returns RGB values)
    const math::Vector4 ConvertToTristimulus(const Wavelength& wavelength) const;
};

RT_FORCE_INLINE const RayColor operator * (const float a, const RayColor& b)
{
    return b * a;
}


#ifndef RT_ENABLE_SPECTRAL_RENDERING

RT_FORCE_INLINE const math::Vector4 RayColor::ConvertToTristimulus(const Wavelength&) const
{
    return value;
}

#endif // RT_ENABLE_SPECTRAL_RENDERING


} // namespace rt
