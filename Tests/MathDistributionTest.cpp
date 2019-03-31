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

TEST(MathTest, Texture_Sample)
{
    std::shared_ptr<Bitmap> bitmap(new Bitmap);
    bitmap->Load("../../../../TEXTURES/wallpaper.bmp");

    std::shared_ptr<BitmapTexture> texture(new BitmapTexture(bitmap));
    texture->MakeSamplable();

    Bitmap outputBitmap;
    outputBitmap.Init(1024, 512, Bitmap::Format::B8G8R8A8_UNorm);
    outputBitmap.Clear();

    Random random;
    for (Uint32 i = 0; i < 10000000; ++i)
    {
        Vector4 coords;
        texture->Sample(random.GetFloat2(), coords, nullptr);
        coords *= Vector4(1024.0f, 512.0f);

        Int32 x = Clamp((Int32)coords.x, 0, 1024 - 1);
        Int32 y = Clamp((Int32)coords.y, 0, 512 - 1);
        Uint32* data = outputBitmap.GetDataAs<Uint32>() + (1024 * y + x);

        *data += 0x010101;
    }

    outputBitmap.SaveBMP("a.bmp", false);
}