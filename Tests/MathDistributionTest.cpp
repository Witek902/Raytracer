#include "PCH.h"
#include "../Core/Math/Distribution.h"
#include "../Core/Math/Random.h"
#include "../Core/Math/SamplingHelpers.h"

#include "../Core/Utils/Bitmap.h"
#include "../Core/Textures/BitmapTexture.h"

using namespace rt;
using namespace rt::math;

//////////////////////////////////////////////////////////////////////////

TEST(MathTest, Distribution_SingleValue)
{
    const float p = 1.0f;
    Distribution distr;
    ASSERT_TRUE(distr.Initialize(&p, 1));

    Random random;

    const uint32 numIterations = 1000;
    for (uint32 i = 0; i < numIterations; ++i)
    {
        float pdf = 0.0f;
        uint32 sample = distr.SampleDiscrete(random.GetFloat(), pdf);

        EXPECT_EQ(1.0f, pdf);
        EXPECT_EQ(0u, sample);
    }
}

TEST(MathTest, Distribution_MultipleValues)
{
    const uint32 pdfSize = 6;
    const uint32 numIterations = 10000;

    const float p[] = { 0.1f, 0.0f, 0.3f, 0.1f, 0.5f, 0.0f };
    Distribution distr;
    ASSERT_TRUE(distr.Initialize(p, pdfSize));

    Random random;

    int32 counters[pdfSize] = { 0,0,0,0 };

    for (uint32 i = 0; i < numIterations; ++i)
    {
        float pdf = 0.0f;
        uint32 sample = distr.SampleDiscrete(random.GetFloat(), pdf);
        ASSERT_LT(sample, pdfSize);

        counters[sample]++;
    }

    for (uint32 i = 0; i < pdfSize; ++i)
    {
        int32 expected = (int32)(numIterations * p[i]);

        EXPECT_LT(Abs(expected - counters[i]), 200);
    }
}
