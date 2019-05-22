#include "PCH.h"
#include "../Core/Math/Distribution.h"
#include "../Core/Math/Random.h"
#include "../Core/Math/SamplingHelpers.h"

#include "../Core/Utils/Bitmap.h"
#include "../Core/Textures/BitmapTexture.h"

#include "gtest/gtest.h"

using namespace rt;
using namespace rt::math;

//////////////////////////////////////////////////////////////////////////

TEST(MathTest, Distribution_SingleValue)
{
    const float p = 1.0f;
    Distribution distr;
    ASSERT_TRUE(distr.Initialize(&p, 1));

    Random random;

    const Uint32 numIterations = 1000;
    for (Uint32 i = 0; i < numIterations; ++i)
    {
        float pdf = 0.0f;
        Uint32 sample = distr.SampleDiscrete(random.GetFloat(), pdf);

        EXPECT_EQ(1.0f, pdf);
        EXPECT_EQ(0, sample);
    }
}

TEST(MathTest, Distribution_MultipleValues)
{
    const Uint32 pdfSize = 6;
    const Uint32 numIterations = 10000;

    const float p[] = { 0.1f, 0.0f, 0.3f, 0.1f, 0.5f, 0.0f };
    Distribution distr;
    ASSERT_TRUE(distr.Initialize(p, pdfSize));

    Random random;

    Int32 counters[pdfSize] = { 0,0,0,0 };

    for (Uint32 i = 0; i < numIterations; ++i)
    {
        float pdf = 0.0f;
        Uint32 sample = distr.SampleDiscrete(random.GetFloat(), pdf);
        ASSERT_LT(sample, pdfSize);

        counters[sample]++;
    }

    for (Uint32 i = 0; i < pdfSize; ++i)
    {
        Int32 expected = (Int32)(numIterations * p[i]);

        EXPECT_TRUE(Abs(expected - counters[i]) < 200u);
    }
}
