#include "PCH.h"
#include "Color.h"

namespace rt {


static const float colorMatchingX[Color::NumBins] =
{
    0.001368f, 0.002236f, 0.004243f, 0.00765f, 0.01431f, 0.02319f, 0.04351f, 0.07763f, 0.13438f, 0.21477f, 0.2839f,
    0.3285f, 0.34828f, 0.34806f, 0.3362f, 0.3187f, 0.2908f, 0.2511f, 0.19536f, 0.1421f, 0.09564f, 0.05795001f, 0.03201f,
    0.0147f, 0.0049f, 0.0024f, 0.0093f, 0.0291f, 0.06327f, 0.1096f, 0.1655f, 0.2257499f, 0.2904f, 0.3597f, 0.4334499f,
    0.5120501f, 0.5945f, 0.6784f, 0.7621f, 0.8425f, 0.9163f, 0.9786f, 1.0263f, 1.0567f, 1.0622f, 1.0456f, 1.0026f, 0.9384f,
    0.8544499f, 0.7514f, 0.6424f, 0.5419f, 0.4479f, 0.3608f, 0.2835f, 0.2187f, 0.1649f, 0.1212f, 0.0874f, 0.0636f, 0.04677f,
    0.0329f, 0.0227f, 0.01584f, 0.01135916f, 0.008110916f, 0.005790346f, 0.004106457f, 0.002899327f, 0.00204919f,
    0.001439971f, 0.0009999493f, 0.0006900786f, 0.0004760213f, 0.0003323011f, 0.0002348261f, 0.0001661505f,
    0.000117413f, 0.00008307527f, 0.00005870652f, 0.00004150994f,
};

static const float colorMatchingY[Color::NumBins] =
{
    0.0000390f, 0.0000640f, 0.000120f, 0.0002170f, 0.0003960f, 0.000640f, 0.001210f, 0.002180f, 0.0040f, 0.00730f,
    0.01160f, 0.016840f, 0.0230f, 0.02980f, 0.0380f, 0.0480f, 0.060f, 0.07390f, 0.090980f, 0.11260f, 0.139020f, 0.16930f,
    0.208020f, 0.25860f, 0.3230f, 0.40730f, 0.5030f, 0.60820f, 0.710f, 0.79320f, 0.8620f, 0.91485010f, 0.9540f, 0.98030f,
    0.99495010f, 1.0f, 0.995f, 0.9786f, 0.952f, 0.9154f, 0.87f, 0.8163f, 0.757f, 0.6949f, 0.631f, 0.5668f, 0.503f, 0.4412f,
    0.381f, 0.321f, 0.265f, 0.217f, 0.175f, 0.1382f, 0.107f, 0.0816f, 0.061f, 0.04458f, 0.032f, 0.0232f, 0.017f, 0.01192f,
    0.00821f, 0.005723f, 0.004102f, 0.002929f, 0.002091f, 0.001484f, 0.001047f, 0.00074f, 0.00052f, 0.0003611f,
    0.0002492f, 0.0001719f, 0.00012f, 0.0000848f, 0.00006f, 0.0000424f, 0.00003f, 0.0000212f, 0.00001499f,
};

static const float colorMatchingZ[Color::NumBins] =
{
    0.006450001f, 0.01054999f, 0.02005001f, 0.03621f, 0.06785001f, 0.1102f, 0.2074f, 0.3713f, 0.6456f, 1.0390501f,
    1.3856f, 1.62296f, 1.74706f, 1.7826f, 1.77211f, 1.7441f, 1.6692f, 1.5281f, 1.28764f, 1.0419f, 0.8129501f, 0.6162f,
    0.46518f, 0.3533f, 0.272f, 0.2123f, 0.1582f, 0.1117f, 0.07824999f, 0.05725001f, 0.04216f, 0.02984f, 0.0203f, 0.0134f,
    0.008749999f, 0.005749999f, 0.0039f, 0.002749999f, 0.0021f, 0.0018f, 0.001650001f, 0.0014f, 0.0011f, 0.001f, 0.0008f,
    0.0006f, 0.00034f, 0.00024f, 0.00019f, 0.0001f, 0.00004999999f, 0.00003f, 0.00002f, 0.00001f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
};

// standard D65 illuminant (daylight)
static const float illuminantD65[Color::NumBins] =
{
    49.9755f, 52.3118f, 54.6482f, 68.7015f, 82.7549f, 87.1204f, 91.486f, 92.4589f, 93.4318f, 90.057f, 86.6823f, 95.7736f, 104.865f,
    110.936f, 117.008f, 117.41f, 117.812f, 116.336f, 114.861f, 115.392f, 115.923f, 112.367f, 108.811f, 109.082f, 109.354f, 108.578f,
    107.802f, 106.296f, 104.79f, 106.239f, 107.689f, 106.047f, 104.405f, 104.225f, 104.046f, 102.023f, 100.0f, 98.1671f, 96.3342f,
    96.0611f, 95.788f, 92.2368f, 88.6856f, 89.3459f, 90.0062f, 89.8026f, 89.5991f, 88.6489f, 87.6987f, 85.4936f, 83.2886f, 83.4939f,
    83.6992f, 81.863f, 80.0268f, 80.1207f, 80.2146f, 81.2462f, 82.2778f, 80.281f, 78.2842f, 74.0027f, 69.7213f, 70.6652f, 71.6091f,
    72.979f, 74.349f, 67.9765f, 61.604f, 65.7448f, 69.8856f, 72.4863f, 75.087f, 69.3398f, 63.5927f, 55.0054f, 46.4182f, 56.6118f,
    66.8054f, 65.0941f, 63.3828f,
};


Color::Color()
    : spectrumSamples{ 0.0f }
{
}

Color Color::BlackBody(const float temperature, float intensity)
{
    Color result;

    const float h = 6.6260693e-34f; // Planck constant
    const float c = 299792458.0f;   // speed of light
    const float k = 1.3806505e-23f; // Boltzmann's constant

    float c1 = intensity * 2.0f * RT_PI * h * c * c;
    float c2 = h * c / k;

    float sum = 0.0f;
    float wavelenght = LowerWavelength;
    for (Uint32 i = 0; i < NumBins; i++)
    {
        const float wavelenghtInMeters = wavelenght * 1e-9f;
        const float power = (c1 / powf(wavelenghtInMeters, 5.0f)) * (1.0f / (expf(c2 / (wavelenghtInMeters * temperature)) - 1.0f));
        result.spectrumSamples[i] = power;
        wavelenght += WavelengthStep;
        sum += power;
    }

    for (Uint32 i = 0; i < NumBins; i++)
    {
        result.spectrumSamples[i] /= sum;
    }

    return result;
}

Color Color::FromSingleWavelength(float wavelength, float intensity)
{
    Color result;

    wavelength -= LowerWavelength;
    Int32 bin = (Int32)(wavelength / WavelengthStep);

    if (bin >= 0 && bin < NumBins)
    {
        const float weight = wavelength / WavelengthStep - (float)bin;
        result.spectrumSamples[bin] = (1.0f - weight) * intensity;
        result.spectrumSamples[bin + 1] = weight * intensity;
    }

    return result;
}

Color Color::Uniform(float intensity)
{
    Color result;

    for (Uint32 i = 0; i < NumBins; i++)
    {
        result.spectrumSamples[i] = intensity;
    }

    return result;
}

math::Vector4 Color::ToXYZ() const
{
    math::Vector4 xyz;

    // integrate spectrum
    float luminanceSum = 0;
    for (int i = 0; i < NumBins; ++i)
    {
        const float illuminant = illuminantD65[i];
        xyz[0] += spectrumSamples[i] * illuminant * colorMatchingX[i];
        xyz[1] += spectrumSamples[i] * illuminant * colorMatchingY[i];
        xyz[2] += spectrumSamples[i] * illuminant * colorMatchingZ[i];
        luminanceSum += illuminant * colorMatchingY[i];
    }
    xyz /= luminanceSum;

    return xyz;
}

math::Vector4 Color::ToRGB() const
{
    return ConvertXYZtoRGB(math::Vector4::Max(math::Vector4(), ToXYZ()));
}


} // namespace rt