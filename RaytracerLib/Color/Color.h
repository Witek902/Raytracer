#pragma once

#include "../RayLib.h"
#include "../Math/Vector4.h"


namespace rt {


// Color represented as spectrum samples
// Used for representing color of lights, materials reflectivity, etc.
class RAYLIB_API Color
{
public:
    static constexpr Uint32 NumBins = 81;

    static constexpr float LowerWavelength = 380.0f;
    static constexpr float HigherWavelength = 780.0f;
    static constexpr float WavelengthStep = 5.0f;

    Color();

    // build from sRGB values
    static Color FromRGB(const float r, const float g, const float b, const Uint32 numSamples = 10);
    static Color FromRGB(const math::Vector4& color, const Uint32 numSamples = 10);

    // generate spectrum of a black body with given temperature (in Kelvins)
    static Color BlackBody(const float temperature, float intensity = 1.0f);

    // generate laser-like spectrum
    static Color FromSingleWavelength(float wavelength, float intensity = 1.0f);

    // set to uniform power spectrum
    static Color Uniform(float intensity = 1.0f);

    // convert to CIE XYZ tristimulus values
    math::Vector4 ToXYZ() const;

    // convert to sRGB tristimulus values
    math::Vector4 ToRGB() const;

    float spectrumSamples[NumBins];
};


// Convert CIE XYZ to sRGB
RT_FORCE_INLINE math::Vector4 ConvertXYZtoRGB(const math::Vector4 xyzColor)
{
    const float mapping[3][3] =
    {
        {  3.2404542f, -1.5371385f, -0.4985314f },
        { -0.9692660f,  1.8760108f,  0.0415560f },
        {  0.0556434f, -0.2040259f,  1.0572252f }
    };

    return math::Vector4(
        mapping[0][0] * xyzColor[0] + mapping[0][1] * xyzColor[1] + mapping[0][2] * xyzColor[2],
        mapping[1][0] * xyzColor[0] + mapping[1][1] * xyzColor[1] + mapping[1][2] * xyzColor[2],
        mapping[2][0] * xyzColor[0] + mapping[2][1] * xyzColor[1] + mapping[2][2] * xyzColor[2]
    );
}


} // namespace rt
