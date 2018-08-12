#pragma once

#include "../RayLib.h"
#include "../Math/Vector4.h"


namespace rt {


// Represents ray wavelength(s), randomized for primary rays
struct RayWavelength
{
    math::Vector4 values;
};


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


// Color represented as spectrum samples
// Used for representing color of lights, materials reflectivity, etc.
class RAYLIB_API Color
{
public:
    static constexpr Uint32 NumBins = 81;
    static constexpr float LowerWavelength = 380.0f;
    static constexpr float HigherWavelength = 780.0f;
    static constexpr float WavelengthStep = 5.0f;

    // calculate ray color values for a black body with given temperature (in Kelvins)
    static RayColor SampleBlackBody(const RayWavelength& wavelength, const float temperature, float intensity = 1.0f);

    // calculate ray color values for given wavelength and linear RGB values
    static RayColor SampleRGB(const RayWavelength& wavelength, const math::Vector4& rgbValues);

    // convert to CIE XYZ tristimulus values
    static math::Vector4 ToXYZ(const RayWavelength& wavelength, const RayColor& value);
};




// Convert CIE XYZ to linear RGB (Rec. BT.709)
RT_FORCE_INLINE math::Vector4 ConvertXYZtoRGB(const math::Vector4 xyzColor)
{
    const float mapping[3][3] =
    {
        { 3.240479f, -1.537150f, -0.498535f },
        { -0.969256f,  1.875991f,  0.041556f },
        { 0.055648f, -0.204043f,  1.057311f }
    };

    return math::Vector4
    (
        mapping[0][0] * xyzColor[0] + mapping[0][1] * xyzColor[1] + mapping[0][2] * xyzColor[2],
        mapping[1][0] * xyzColor[0] + mapping[1][1] * xyzColor[1] + mapping[1][2] * xyzColor[2],
        mapping[2][0] * xyzColor[0] + mapping[2][1] * xyzColor[1] + mapping[2][2] * xyzColor[2],
        0.0f
    );
}

// Convert linear RGB (Rec. BT.709) to CIE XYZ
RT_FORCE_INLINE math::Vector4 ConvertRGBtoXYZ(const math::Vector4 rgbColor)
{
    const float mapping[3][3] =
    {
        { 0.412453f, 0.357580f, 0.180423f },
        { 0.212671f, 0.715160f, 0.072169f },
        { 0.019334f, 0.119193f, 0.950227f }
    };

    return math::Vector4
    (
        mapping[0][0] * rgbColor[0] + mapping[0][1] * rgbColor[1] + mapping[0][2] * rgbColor[2],
        mapping[1][0] * rgbColor[0] + mapping[1][1] * rgbColor[1] + mapping[1][2] * rgbColor[2],
        mapping[2][0] * rgbColor[0] + mapping[2][1] * rgbColor[1] + mapping[2][2] * rgbColor[2],
        0.0f
    );
}

// Convert HSV to linear RGB
RT_FORCE_INLINE math::Vector4 HSVtoRGB(const Float hue, const Float saturation, const Float value)
{
    const int h_i = (int)(hue * 6.0f);
    const float f = hue * 6 - h_i;
    const float p = value * (1 - saturation);
    const float q = value * (1 - f * saturation);
    const float t = value * (1 - (1 - f) * saturation);

    if (h_i == 0)
        return math::Vector4(value, t, p, 0.0f);
    else if (h_i == 1)
        return math::Vector4(q, value, p, 0.0f);
    else if (h_i == 2)
        return math::Vector4(p, value, t, 0.0f);
    else if (h_i == 3)
        return math::Vector4(p, q, value, 0.0f);
    else if (h_i == 4)
        return math::Vector4(t, p, value, 0.0f);
    else if (h_i == 5)
        return math::Vector4(value, p, q, 0.0f);

    return math::Vector4();
}

} // namespace rt
