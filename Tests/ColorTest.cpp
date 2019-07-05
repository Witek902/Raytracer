#include "PCH.h"
/*
#include "../Core/Color/Color.h"
#include "../Core/Color/ColorHelpers.h"
#include "../Core/Math/Random.h"

using namespace rt;
using namespace math;

class ColorTestFixture : public ::testing::Test
{
public:
    ColorTestFixture() = default;

};

//////////////////////////////////////////////////////////////////////////

static void TestColor(const math::Vector4& originalRGB)
{
    math::Random random;
    const size_t maxIterations = 20;

    Wavelength wavelength;
    math::Vector4 xyzSum;
    for (size_t i = 0; i < maxIterations; ++i)
    {
        wavelength.Randomize(random);
        Color color = Color::SampleRGB(wavelength, originalRGB);
        xyzSum += color.ToXYZ(wavelength);
    }

    xyzSum /= (float)maxIterations;

    const math::Vector4 calculatedRGB = ConvertXYZtoRGB(xyzSum);

    EXPECT_NEAR(originalRGB.x, calculatedRGB.x, 0.05f);
    EXPECT_NEAR(originalRGB.y, calculatedRGB.y, 0.05f);
    EXPECT_NEAR(originalRGB.z, calculatedRGB.z, 0.05f);
}


TEST_F(ColorTestFixture, SpectrumToRGB)
{
    TestColor(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
    TestColor(Vector4(0.0f, 0.0f, 1.0f, 0.0f));
    TestColor(Vector4(0.0f, 1.0f, 0.0f, 0.0f));
    TestColor(Vector4(0.0f, 1.0f, 1.0f, 0.0f));
    TestColor(Vector4(1.0f, 0.0f, 0.0f, 0.0f));
    TestColor(Vector4(1.0f, 0.0f, 1.0f, 0.0f));
    TestColor(Vector4(1.0f, 1.0f, 0.0f, 0.0f));
    TestColor(Vector4(1.0f, 1.0f, 1.0f, 0.0f));
}
*/
