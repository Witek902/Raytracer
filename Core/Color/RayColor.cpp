#include "PCH.h"
#include "RayColor.h"
#include "Spectrum.h"

#ifdef _MSC_VER
#pragma warning(disable: 4305) // truncation from 'double' to 'const float'
#endif

namespace rt {

using namespace math;

static_assert(sizeof(RayColor) == sizeof(float) * Wavelength::NumComponents, "Invalid number of color components");

#ifdef RT_ENABLE_SPECTRAL_RENDERING

static constexpr Uint32 NumBins = 69;

static const float colorMatchingX[NumBins] =
{
    0.001368f, 0.002236f, 0.004243f, 0.00765f, 0.01431f, 0.02319f, 0.04351f, 0.07763f, 0.13438f, 0.21477f, 0.2839f,
    0.3285f, 0.34828f, 0.34806f, 0.3362f, 0.3187f, 0.2908f, 0.2511f, 0.19536f, 0.1421f, 0.09564f, 0.05795001f, 0.03201f,
    0.0147f, 0.0049f, 0.0024f, 0.0093f, 0.0291f, 0.06327f, 0.1096f, 0.1655f, 0.2257499f, 0.2904f, 0.3597f, 0.4334499f,
    0.5120501f, 0.5945f, 0.6784f, 0.7621f, 0.8425f, 0.9163f, 0.9786f, 1.0263f, 1.0567f, 1.0622f, 1.0456f, 1.0026f, 0.9384f,
    0.8544499f, 0.7514f, 0.6424f, 0.5419f, 0.4479f, 0.3608f, 0.2835f, 0.2187f, 0.1649f, 0.1212f, 0.0874f, 0.0636f, 0.04677f,
    0.0329f, 0.0227f, 0.01584f, 0.01135916f, 0.008110916f, 0.005790346f, 0.004106457f, 0.002899327f
};

static const float colorMatchingY[NumBins] =
{
    0.0000390f, 0.0000640f, 0.000120f, 0.0002170f, 0.0003960f, 0.000640f, 0.001210f, 0.002180f, 0.0040f, 0.00730f,
    0.01160f, 0.016840f, 0.0230f, 0.02980f, 0.0380f, 0.0480f, 0.060f, 0.07390f, 0.090980f, 0.11260f, 0.139020f, 0.16930f,
    0.208020f, 0.25860f, 0.3230f, 0.40730f, 0.5030f, 0.60820f, 0.710f, 0.79320f, 0.8620f, 0.91485010f, 0.9540f, 0.98030f,
    0.99495010f, 1.0f, 0.995f, 0.9786f, 0.952f, 0.9154f, 0.87f, 0.8163f, 0.757f, 0.6949f, 0.631f, 0.5668f, 0.503f, 0.4412f,
    0.381f, 0.321f, 0.265f, 0.217f, 0.175f, 0.1382f, 0.107f, 0.0816f, 0.061f, 0.04458f, 0.032f, 0.0232f, 0.017f, 0.01192f,
    0.00821f, 0.005723f, 0.004102f, 0.002929f, 0.002091f, 0.001484f, 0.001047f
};

static const float colorMatchingZ[NumBins] =
{
    0.006450001f, 0.01054999f, 0.02005001f, 0.03621f, 0.06785001f, 0.1102f, 0.2074f, 0.3713f, 0.6456f, 1.0390501f,
    1.3856f, 1.62296f, 1.74706f, 1.7826f, 1.77211f, 1.7441f, 1.6692f, 1.5281f, 1.28764f, 1.0419f, 0.8129501f, 0.6162f,
    0.46518f, 0.3533f, 0.272f, 0.2123f, 0.1582f, 0.1117f, 0.07824999f, 0.05725001f, 0.04216f, 0.02984f, 0.0203f, 0.0134f,
    0.008749999f, 0.005749999f, 0.0039f, 0.002749999f, 0.0021f, 0.0018f, 0.001650001f, 0.0014f, 0.0011f, 0.001f, 0.0008f,
    0.0006f, 0.00034f, 0.00024f, 0.00019f, 0.0001f, 0.00004999999f, 0.00003f, 0.00002f, 0.00001f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
};

static const float colorMatchinhNormFactor = 1.0f / 21.3689137f; // sum of colorMatchingY

// standard D65 illuminant (daylight)
static const float illuminantD65[NumBins] =
{
    49.9755f, 52.3118f, 54.6482f, 68.7015f, 82.7549f, 87.1204f, 91.486f, 92.4589f, 93.4318f, 90.057f, 86.6823f, 95.7736f, 104.865f,
    110.936f, 117.008f, 117.41f, 117.812f, 116.336f, 114.861f, 115.392f, 115.923f, 112.367f, 108.811f, 109.082f, 109.354f, 108.578f,
    107.802f, 106.296f, 104.79f, 106.239f, 107.689f, 106.047f, 104.405f, 104.225f, 104.046f, 102.023f, 100.0f, 98.1671f, 96.3342f,
    96.0611f, 95.788f, 92.2368f, 88.6856f, 89.3459f, 90.0062f, 89.8026f, 89.5991f, 88.6489f, 87.6987f, 85.4936f, 83.2886f, 83.4939f,
    83.6992f, 81.863f, 80.0268f, 80.1207f, 80.2146f, 81.2462f, 82.2778f, 80.281f, 78.2842f, 74.0027f, 69.7213f, 70.6652f, 71.6091f,
    72.979f, 74.349f, 67.9765f, 61.604f
};

static constexpr int rgbToSpectrumNumSamples = 32;

const float rgbToSpectrum_White[rgbToSpectrumNumSamples] =
{
    1.0618958571272863e+00f,   1.0615019980348779e+00f,    1.0614335379927147e+00f,   1.0622711654692485e+00f,
    1.0622036218416742e+00f,   1.0625059965187085e+00f,    1.0623938486985884e+00f,   1.0624706448043137e+00f,
    1.0625048144827762e+00f,   1.0624366131308856e+00f,    1.0620694238892607e+00f,   1.0613167586932164e+00f,
    1.0610334029377020e+00f,   1.0613868564828413e+00f,    1.0614215366116762e+00f,   1.0620336151299086e+00f,
    1.0625497454805051e+00f,   1.0624317487992085e+00f,    1.0625249140554480e+00f,   1.0624277664486914e+00f,
    1.0624749854090769e+00f,   1.0625538581025402e+00f,    1.0625326910104864e+00f,   1.0623922312225325e+00f,
    1.0623650980354129e+00f,   1.0625256476715284e+00f,    1.0612277619533155e+00f,   1.0594262608698046e+00f,
    1.0599810758292072e+00f,   1.0602547314449409e+00f,    1.0601263046243634e+00f,   1.0606565756823634e+00f
};

const float rgbToSpectrum_Cyan[rgbToSpectrumNumSamples] =
{
     1.0414628021426751e+00f,   1.0328661533771188e+00f,    1.0126146228964314e+00f,   1.0350460524836209e+00f,
     1.0078661447098567e+00f,   1.0422280385081280e+00f,    1.0442596738499825e+00f,   1.0535238290294409e+00f,
     1.0180776226938120e+00f,   1.0442729908727713e+00f,    1.0529362541920750e+00f,   1.0537034271160244e+00f,
     1.0533901869215969e+00f,   1.0537782700979574e+00f,    1.0527093770467102e+00f,   1.0530449040446797e+00f,
     1.0550554640191208e+00f,   1.0553673610724821e+00f,    1.0454306634683976e+00f,   6.2348950639230805e-01f,
     1.8038071613188977e-01f,  -7.6303759201984539e-03f,   -1.5217847035781367e-04f,  -7.5102257347258311e-03f,
    -2.1708639328491472e-03f,   6.5919466602369636e-04f,    1.2278815318539780e-02f,  -4.4669775637208031e-03f,
     1.7119799082865147e-02f,   4.9211089759759801e-03f,    5.8762925143334985e-03f,   2.5259399415550079e-02f
};

const float rgbToSpectrum_Magenta[rgbToSpectrumNumSamples] =
{
    9.9422138151236850e-01f,   9.8986937122975682e-01f,    9.8293658286116958e-01f,   9.9627868399859310e-01f,
    1.0198955019000133e+00f,   1.0166395501210359e+00f,    1.0220913178757398e+00f,   9.9651666040682441e-01f,
    1.0097766178917882e+00f,   1.0215422470827016e+00f,    6.4031953387790963e-01f,   2.5012379477078184e-03f,
    6.5339939555769944e-03f,   2.8334080462675826e-03f,   -5.1209675389074505e-11f,  -9.0592291646646381e-03f,
    3.3936718323331200e-03f,  -3.0638741121828406e-03f,    2.2203936168286292e-01f,   6.3141140024811970e-01f,
    9.7480985576500956e-01f,   9.7209562333590571e-01f,    1.0173770302868150e+00f,   9.9875194322734129e-01f,
    9.4701725739602238e-01f,   8.5258623154354796e-01f,    9.4897798581660842e-01f,   9.4751876096521492e-01f,
    9.9598944191059791e-01f,   8.6301351503809076e-01f,    8.9150987853523145e-01f,   8.4866492652845082e-01f
};

const float rgbToSpectrum_Yellow[rgbToSpectrumNumSamples] =
{
     5.5740622924920873e-03f,  -4.7982831631446787e-03f,   -5.2536564298613798e-03f,  -6.4571480044499710e-03f,
    -5.9693514658007013e-03f,  -2.1836716037686721e-03f,    1.6781120601055327e-02f,   9.6096355429062641e-02f,
     2.1217357081986446e-01f,   3.6169133290685068e-01f,    5.3961011543232529e-01f,   7.4408810492171507e-01f,
     9.2209571148394054e-01f,   1.0460304298411225e+00f,    1.0513824989063714e+00f,   1.0511991822135085e+00f,
     1.0510530911991052e+00f,   1.0517397230360510e+00f,    1.0516043086790485e+00f,   1.0511944032061460e+00f,
     1.0511590325868068e+00f,   1.0516612465483031e+00f,    1.0514038526836869e+00f,   1.0515941029228475e+00f,
     1.0511460436960840e+00f,   1.0515123758830476e+00f,    1.0508871369510702e+00f,   1.0508923708102380e+00f,
     1.0477492815668303e+00f,   1.0493272144017338e+00f,    1.0435963333422726e+00f,   1.0392280772051465e+00f
};

const float rgbToSpectrum_Red[rgbToSpectrumNumSamples] =
{
     1.6575604867086180e-01f,   1.1846442802747797e-01f,    1.2408293329637447e-01f,   1.1371272058349924e-01f,
     7.8992434518899132e-02f,   3.2205603593106549e-02f,   -1.0798365407877875e-02f,   1.8051975516730392e-02f,
     5.3407196598730527e-03f,   1.3654918729501336e-02f,   -5.9564213545642841e-03f,  -1.8444365067353252e-03f,
    -1.0571884361529504e-02f,  -2.9375521078000011e-03f,   -1.0790476271835936e-02f,  -8.0224306697503633e-03f,
    -2.2669167702495940e-03f,   7.0200240494706634e-03f,   -8.1528469000299308e-03f,   6.0772866969252792e-01f,
     9.8831560865432400e-01f,   9.9391691044078823e-01f,    1.0039338994753197e+00f,   9.9234499861167125e-01f,
     9.9926530858855522e-01f,   1.0084621557617270e+00f,    9.8358296827441216e-01f,   1.0085023660099048e+00f,
     9.7451138326568698e-01f,   9.8543269570059944e-01f,    9.3495763980962043e-01f,   9.8713907792319400e-01f
};

const float rgbToSpectrum_Green[rgbToSpectrumNumSamples] =
{
     2.6494153587602255e-03f,  -5.0175013429732242e-03f,   -1.2547236272489583e-02f,  -9.4554964308388671e-03f,
    -1.2526086181600525e-02f,  -7.9170697760437767e-03f,   -7.9955735204175690e-03f,  -9.3559433444469070e-03f,
     6.5468611982999303e-02f,   3.9572875517634137e-01f,    7.5244022299886659e-01f,   9.6376478690218559e-01f,
     9.9854433855162328e-01f,   9.9992977025287921e-01f,    9.9939086751140449e-01f,   9.9994372267071396e-01f,
     9.9939121813418674e-01f,   9.9911237310424483e-01f,    9.6019584878271580e-01f,   6.3186279338432438e-01f,
     2.5797401028763473e-01f,   9.4014888527335638e-03f,   -3.0798345608649747e-03f,  -4.5230367033685034e-03f,
    -6.8933410388274038e-03f,  -9.0352195539015398e-03f,   -8.5913667165340209e-03f,  -8.3690869120289398e-03f,
    -7.8685832338754313e-03f,  -8.3657578711085132e-06f,    5.4301225442817177e-03f,  -2.7745589759259194e-03f
};

const float rgbToSpectrum_Blue[rgbToSpectrumNumSamples] =
{
     9.9209771469720676e-01f,   9.8876426059369127e-01f,    9.9539040744505636e-01f,   9.9529317353008218e-01f,
     9.9181447411633950e-01f,   1.0002584039673432e+00f,    9.9968478437342512e-01f,   9.9988120766657174e-01f,
     9.8504012146370434e-01f,   7.9029849053031276e-01f,    5.6082198617463974e-01f,   3.3133458513996528e-01f,
     1.3692410840839175e-01f,   1.8914906559664151e-02f,   -5.1129770932550889e-06f,  -4.2395493167891873e-04f,
    -4.1934593101534273e-04f,   1.7473028136486615e-03f,    3.7999160177631316e-03f,  -5.5101474906588642e-04f,
    -4.3716662898480967e-05f,   7.5874501748732798e-03f,    2.5795650780554021e-02f,   3.8168376532500548e-02f,
     4.9489586408030833e-02f,   4.9595992290102905e-02f,    4.9814819505812249e-02f,   3.9840911064978023e-02f,
     3.0501024937233868e-02f,   2.1243054765241080e-02f,    6.9596532104356399e-03f,   4.1733649330980525e-03f
};

/////////////////////////////////////////////////////////////////////////////////

RayColor SampleSpectrum(const float* data, const Uint32 numValues, const Wavelength& wavelength)
{
    const Vector8 scaledWavelengths = wavelength.value * static_cast<float>(numValues - 1);
    const VectorInt8 indices = VectorInt8::Convert(scaledWavelengths);
    const Vector8 weights = scaledWavelengths - indices.ConvertToFloat();

    const Vector8 a = _mm256_i32gather_ps(data, indices, 4);
    const Vector8 b = _mm256_i32gather_ps(data + 1, indices, 4);

    RayColor result;
    result.value = Vector8::Lerp(a, b, weights);

    /*
    for (Uint32 i = 0; i < Wavelength::NumComponents; ++i)
    {
        assert(wavelength.value[i] >= 0.0f);
        assert(wavelength.value[i] < 1.0f);

        const float w = wavelength.value[i] * static_cast<float>(numValues - 1);
        const Uint32 index = static_cast<Uint32>(w);
        assert(index >= 0);
        assert(index + 1 < numValues);

        const float weight = w - static_cast<float>(index);
        result.value[i] = Lerp(data[index], data[index + 1], weight);
    }
    */

    return result;
}

/////////////////////////////////////////////////////////////////////////////////

const RayColor RayColor::BlackBody(const Wavelength& wavelength, const float temperature)
{
    RayColor result;

    using namespace constants;
    double c1 = 2.0f * constants::h * constants::c * constants::c;
    double c2 = h * c / k;

    for (Uint32 i = 0; i < Wavelength::NumComponents; i++)
    {
        // wavelenght in meters
        const double lambda = Wavelength::Lower + (Wavelength::Higher - Wavelength::Lower) * wavelength.value[i];

        // Planck's law equation
        const double term1 = c1 / (lambda * lambda * lambda * lambda * lambda);
        const double term2 = exp(c2 / (lambda * temperature)) - 1.0f;
        result.value[i] = (float)(term1 / term2);
    }

    return result;
}

const RayColor RayColor::Resolve(const Wavelength& wavelength, const Spectrum& spectrum)
{
    const float r = spectrum.rgbValues.x;
    const float g = spectrum.rgbValues.y;
    const float b = spectrum.rgbValues.z;

    float coeffA, coeffB, coeffC;
    const float* sourceB;
    const float* sourceC;

    if (r <= g && r <= b)
    {
        coeffA = r;
        sourceB = rgbToSpectrum_Cyan;
        if (g <= b)
        {
            sourceC = rgbToSpectrum_Blue;
            coeffB = g - r;
            coeffC = b - g;
        }
        else
        {
            sourceC = rgbToSpectrum_Green;
            coeffB = b - r;
            coeffC = g - b;
        }
    }
    else if (g <= r && g <= b)
    {
        coeffA = g;
        sourceB = rgbToSpectrum_Magenta;
        if (r <= b)
        {
            sourceC = rgbToSpectrum_Blue;
            coeffB = r - g;
            coeffC = b - r;
        }
        else
        {
            sourceC = rgbToSpectrum_Red;
            coeffB = b - g;
            coeffC = r - b;
        }
    }
    else
    {
        coeffA = b;
        sourceB = rgbToSpectrum_Yellow;
        if (r <= g)
        {
            sourceC = rgbToSpectrum_Green;
            coeffB = r - b;
            coeffC = g - r;
        }
        else
        {
            sourceC = rgbToSpectrum_Red;
            coeffB = g - b;
            coeffC = r - g;
        }
    }

    RayColor result;
    result = SampleSpectrum(rgbToSpectrum_White, rgbToSpectrumNumSamples, wavelength) * coeffA;
    result += SampleSpectrum(sourceB, rgbToSpectrumNumSamples, wavelength) * coeffB;
    result += SampleSpectrum(sourceC, rgbToSpectrumNumSamples, wavelength) * coeffC;
    return result * 0.86445f;
}

const Vector4 RayColor::ConvertToTristimulus(const Wavelength& wavelength) const
{
    Vector3x8 xyz;
    RayColor illuminant = SampleSpectrum(illuminantD65, NumBins, wavelength);
    xyz.x = SampleSpectrum(colorMatchingX, NumBins, wavelength).value;
    xyz.y = SampleSpectrum(colorMatchingY, NumBins, wavelength).value;
    xyz.z = SampleSpectrum(colorMatchingZ, NumBins, wavelength).value;
    xyz *= value * illuminant.value;

    Vector4 v[8];
    xyz.Unpack(v);

    Vector4 result = ((v[0] + v[1]) + (v[2] + v[3])) + ((v[4] + v[5]) + (v[6] + v[7]));
    result *= 1.0f / 1.33f;
    result *= colorMatchinhNormFactor / static_cast<float>(Wavelength::NumComponents);
    return result;
}

#else // !RT_ENABLE_SPECTRAL_RENDERING

const RayColor RayColor::Resolve(const Wavelength&, const Spectrum& spectrum)
{
    return RayColor{ spectrum.rgbValues };
}

#endif // RT_ENABLE_SPECTRAL_RENDERING

} // namespace rt