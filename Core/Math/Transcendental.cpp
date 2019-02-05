#include "PCH.h"
#include "Transcendental.h"
#include "VectorInt8.h"

namespace rt {
namespace math {

namespace {

RT_FORCE_INLINE Int32 FloatAsInt(const float f)
{
    Bits32 bits;
    bits.f = f;
    return bits.si;
}

RT_FORCE_INLINE float IntAsFloat(const Int32 i)
{
    Bits32 bits;
    bits.si = i;
    return bits.f;
}

} // namespace

float Sin(float x)
{
    // based on:
    // https://www.gamedev.net/forums/topic/681723-faster-sin-and-cos/

    // range reduction
    const Int32 i = static_cast<Int32>(x * (1.0f / RT_PI));
    x -= static_cast<float>(i) * RT_PI;

    const float x2 = x * x;

    const float c0 =  9.9999970197e-01f;
    const float c1 = -1.6666577756e-01f;
    const float c2 =  8.3325579762e-03f;
    const float c3 = -1.9812576647e-04f;
    const float c4 =  2.7040521217e-06f;
    const float c5 = -2.0532988642e-08f;

    float y = x * (c0 + x2 * (c1 + x2 * (c2 + x2 * (c3 + x2 * (c4 + x2 * c5)))));

    return (i & 1) ? -y : y;
}

const Vector4 Sin(Vector4 x)
{
    // based on:
    // https://www.gamedev.net/forums/topic/681723-faster-sin-and-cos/

    // range reduction
    const VectorInt4 i = VectorInt4::Convert(x * (1.0f / RT_PI));
    x = Vector4::NegMulAndAdd(i.ConvertToFloat(), RT_PI, x);

    const Vector4 x2 = x * x;

    const Vector4 c0 = Vector4( 9.9999970197e-01f);
    const Vector4 c1 = Vector4(-1.6666577756e-01f);
    const Vector4 c2 = Vector4( 8.3325579762e-03f);
    const Vector4 c3 = Vector4(-1.9812576647e-04f);
    const Vector4 c4 = Vector4( 2.7040521217e-06f);
    const Vector4 c5 = Vector4(-2.0532988642e-08f);

    Vector4 y = Vector4::MulAndAdd(c5, x2, c4);
    y = Vector4::MulAndAdd(y, x2, c3);
    y = Vector4::MulAndAdd(y, x2, c2);
    y = Vector4::MulAndAdd(y, x2, c1);
    y = Vector4::MulAndAdd(y, x2, c0);
    y *= x;

    // equivalent of: (i & 1) ? -y : y;
    return y ^ (i << 31).CastToFloat();
}

const Vector8 Sin(Vector8 x)
{
#ifdef RT_USE_AVX2
    // based on:
    // https://www.gamedev.net/forums/topic/681723-faster-sin-and-cos/

    // range reduction
    const VectorInt8 i = VectorInt8::Convert(x * (1.0f / RT_PI));
    x = Vector8::NegMulAndAdd(i.ConvertToFloat(), RT_PI, x);

    const Vector8 x2 = x * x;

    const Vector8 c0 = Vector8( 9.9999970197e-01f);
    const Vector8 c1 = Vector8(-1.6666577756e-01f);
    const Vector8 c2 = Vector8( 8.3325579762e-03f);
    const Vector8 c3 = Vector8(-1.9812576647e-04f);
    const Vector8 c4 = Vector8( 2.7040521217e-06f);
    const Vector8 c5 = Vector8(-2.0532988642e-08f);

    Vector8 y = Vector8::MulAndAdd(c5, x2, c4);
    y = Vector8::MulAndAdd(y, x2, c3);
    y = Vector8::MulAndAdd(y, x2, c2);
    y = Vector8::MulAndAdd(y, x2, c1);
    y = Vector8::MulAndAdd(y, x2, c0);
    y *= x;

    // equivalent of: (i & 1) ? -y : y;
    return y ^ (i << 31).CastToFloat();
#else
    return Vector8{sinf(x[0]), sinf(x[1]), sinf(x[2]), sinf(x[3]), sinf(x[4]), sinf(x[5]), sinf(x[6]), sinf(x[7])};
#endif // RT_USE_AVX2
}

float Cos(float x)
{
    return Sin(x + RT_PI / 2.0f);
}

const Vector4 Cos(Vector4 x)
{
    return Sin(x + Vector4(RT_PI / 2.0f));
}

const Vector8 Cos(Vector8 x)
{
    return Sin(x + Vector8(RT_PI / 2.0f));
}

float FastACos(float x)
{
    // based on:
    // https://stackoverflow.com/a/26030435/10061517

    float negate = float(x < 0);
    x = fabsf(x);
    float ret = -0.0187293f;
    ret = ret * x + 0.0742610f;
    ret = ret * x - 0.2121144f;
    ret = ret * x + 1.5707288f;
    ret = ret * sqrtf(1.0f - x);
    ret = ret - 2.0f * negate * ret;
    return negate * 3.14159265358979f + ret;
}

float FastExp(float x)
{
    // implementation based on: "A more accurate, performance-competitive implementation of expf" by njuffa

    // handle special cases: severe overflow / underflow
    if (x >= 87.0f) // overflow
    {
        return std::numeric_limits<float>::infinity();
    }
    else if (x <= -87.0f) // underflow
    {
        return 0.0f;
    }

    const float t = x * 1.442695041f;
    const float fi = floorf(t);
    const Int32 i = (Int32)fi;
    const float f = t - fi;

    Bits32 bits;
    bits.f = (0.3371894346f * f + 0.657636276f) * f + 1.00172476f;
    bits.si += (i << 23);
    return bits.f;
}

float Log(float x)
{
    // based on:
    // https://stackoverflow.com/questions/39821367/very-fast-logarithm-natural-log-function-in-c

    // range reduction
    const Int32 e = (FloatAsInt(x) - 0x3f2aaaab) & 0xff800000;
    const float m = IntAsFloat(FloatAsInt(x) - e);
    const float i = 1.19209290e-7f * (float)e;

    const float f = m - 1.0f;
    const float s = f * f;

    // Compute log1p(f) for f in [-1/3, 1/3]
    float r = -0.130187988f * f + 0.140889585f;
    float t = -0.121489584f * f + 0.139809534f;
    r = r * s + t;
    r = r * f  -0.166845024f;
    r = r * f + 0.200121149f;
    r = r * f - 0.249996364f;
    r = r * f + 0.333331943f;
    r = r * f - 0.500000000f;
    r = r * s + f;
    r = i * 0.693147182f + r; // log(2)

    return r;
}

float FastLog(float x)
{
    // based on:
    // https://stackoverflow.com/questions/39821367/very-fast-logarithm-natural-log-function-in-c

    // range reduction
    const Int32 e = (FloatAsInt(x) - 0x3f2aaaab) & 0xff800000;
    const float m = IntAsFloat(FloatAsInt(x) - e);
    const float i = 1.19209290e-7f * (float)e;

    const float f = m - 1.0f;
    const float s = f * f;

    // Compute log1p(f) for f in [-1/3, 1/3]
    float r = 0.230836749f * f - 0.279208571f;
    float t = 0.331826031f * f - 0.498910338f;
    r = r * s + t;
    r = r * s + f;
    r = i * 0.693147182f + r; // log(2)

    return r;
}

float FastATan2(const float y, const float x)
{
    // https://stackoverflow.com/questions/46210708/atan2-approximation-with-11bits-in-mantissa-on-x86with-sse2-and-armwith-vfpv4

    const float ax = math::Abs(x);
    const float ay = math::Abs(y);
    const float mx = math::Max(ay, ax);
    const float mn = math::Min(ay, ax);
    const float a = mn / mx;

    // Minimax polynomial approximation to atan(a) on [0,1]
    const float s = a * a;
    const float c = s * a;
    const float q = s * s;
    const float t = -0.094097948f * q - 0.33213072f;
    float r = (0.024840285f * q + 0.18681418f);
    r = r * s + t;
    r = r * c + a;

    // Map to full circle
    if (ay > ax) r = 1.57079637f - r;
    if (x < 0.0f) r = RT_PI - r;
    if (y < 0.0f) r = -r;
    return r;
}

} // namespace math
} // namespace rt
