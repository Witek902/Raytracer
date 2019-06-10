#pragma once

namespace rt {
namespace math {


float Cos(float x)
{
    return Sin(x + RT_PI / 2.0f);
}

const Vector4 Cos(const Vector4& x)
{
    return Sin(x + Vector4(RT_PI / 2.0f));
}

const Vector8 Cos(const Vector8& x)
{
    return Sin(x + Vector8(RT_PI / 2.0f));
}

const Vector4 SinCos(const float x)
{
    const Vector4 offset(0.0f, RT_PI / 2.0f, 0.0f, 0.0f);
    return Sin(Vector4(x) + offset) & Vector4::MakeMask<1, 1, 0, 0>();
}

} // namespace math
} // namespace rt
