#pragma once

#include "../RayLib.h"
#include "../Math/Vector4.h"


namespace rt {

RT_GLOBAL_CONST math::Vector4 c_rgbIntensityWeights = { 0.21f, 0.72f, 0.07f, 0.0f };

RT_GLOBAL_CONST math::Vector4 XYZtoRGB_r = {  3.240479f, -1.537150f, -0.498535f, 0.0f };
RT_GLOBAL_CONST math::Vector4 XYZtoRGB_g = { -0.969256f,  1.875991f,  0.041556f, 0.0f };
RT_GLOBAL_CONST math::Vector4 XYZtoRGB_b = {  0.055648f, -0.204043f,  1.057311f, 0.0f };

// Convert linear to sRGB
template<typename T>
RT_FORCE_INLINE math::Vector4 Convert_sRGB_To_Linear(const T& gammaColor)
{
    // based on:
    // http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html

    T result;
    result = T::MulAndAdd(gammaColor, T(0.305306011f), T(0.682171111f));
    result = T::MulAndAdd(gammaColor, result, T(0.012522878f));
    result *= gammaColor;
    return result;
}

// Convert sRGB to linear
template<typename T>
RT_FORCE_INLINE math::Vector4 Convert_Linear_To_sRGB(const T& linearColor)
{
    // based on:
    // http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html

    const T s1 = T::Sqrt(linearColor);
    const T s2 = T::Sqrt(s1);
    const T s3 = T::Sqrt(s2);

    T result = 0.585122381f * s1;
    result = T::MulAndAdd(s2, 0.783140355f, result);
    result = T::MulAndAdd(s3, -0.368262736f, result);
    return T::Saturate(result);
}

// Convert CIE XYZ to linear RGB (Rec. BT.709)
RT_FORCE_INLINE math::Vector4 ConvertXYZtoRGB(const math::Vector4& xyzColor)
{
    math::Vector4 r = XYZtoRGB_r * xyzColor;
    math::Vector4 g = XYZtoRGB_g * xyzColor;
    math::Vector4 b = XYZtoRGB_b * xyzColor;

    math::Vector4::Transpose3(r, g, b);

    return r + g + b;
}

// Convert linear RGB (Rec. BT.709) to CIE XYZ
RT_FORCE_INLINE math::Vector4 ConvertRGBtoXYZ(const math::Vector4& rgbColor)
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

enum class Tonemapper : Uint8
{
    Clamped,
    Reinhard,
    JimHejland_RichardBurgessDawson,
    ACES
};

template<typename T>
RT_FORCE_INLINE static const T ToneMap(const T color, const Tonemapper tonemapper)
{
    T result;

    switch (tonemapper)
    {
    case Tonemapper::Clamped:
    {
        result = Convert_Linear_To_sRGB(color);
        break;
    }
    case Tonemapper::Reinhard:
    {
        result = Convert_Linear_To_sRGB(color / (T(1.0f) + color));
        break;
    }
    case Tonemapper::JimHejland_RichardBurgessDawson:
    {
        const T b = T(6.2f);
        const T c = T(1.7f);
        const T d = T(0.06f);
        const T t0 = color * T::MulAndAdd(color, b, T(0.5f));
        const T t1 = T::MulAndAdd(color, b, c);
        const T t2 = T::MulAndAdd(color, t1, d);
        result = t0 * T::FastReciprocal(t2);
        break;
    }
    case Tonemapper::ACES:
    {
        const T a = T(2.51f);
        const T b = T(0.03f);
        const T c = T(2.43f);
        const T d = T(0.59f);
        const T e = T(0.14f);
        const T t0 = color * T::MulAndAdd(color, a, b);
        const T t1 = T::MulAndAdd(color, c, d);
        const T t2 = T::MulAndAdd(color, t1, e);
        result = Convert_Linear_To_sRGB(t0 * T::FastReciprocal(t2));
        break;
    }
    default:
        RT_FATAL("Invalid tonemapper");
    };

    return result;
}

// Convert HSV to linear RGB
RT_FORCE_INLINE math::Vector4 HSVtoRGB(const float hue, const float saturation, const float value)
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

    return math::Vector4::Zero();
}


} // namespace rt
