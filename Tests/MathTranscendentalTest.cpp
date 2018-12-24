#include "PCH.h"
#include "../Core/Math/Transcendental.h"
#include "../Core/Math/Math.h"
#include "../Core/Utils/Logger.h"

#include "gtest/gtest.h"

#include <functional>

using namespace rt;

namespace {

struct TestRange
{
    enum class StepType
    {
        Increment,
        Multiply
    };

    float min;
    float max;
    float step;
    StepType type;

    TestRange(float min, float max, float step, StepType type)
        : min(min), max(max), step(step), type(type)
    { }
};

using TranscendentalFloatFunc = float(*)(float);

void TestTranscendental(const char* name, const TestRange& range,
                        const std::function<float(float)>& func,
                        const TranscendentalFloatFunc& ref,
                        const float maxAbsError, const float maxRelError)
{
    float measuredMaxAbsError = 0.0f;
    float measuredMaxRelError = 0.0f;

    for (float x = range.min; x < range.max; )
    {
        const float calculated = func(x);
        const float reference = ref(x);

        const float relError = math::Abs((calculated - reference) / reference);
        const float absError = math::Abs(calculated - reference);

        EXPECT_LE(relError, maxRelError) << name << "\nCalculated: " << calculated << "\nReference:  " << reference << "\nx = " << x;
        EXPECT_LE(absError, maxAbsError) << name << "\nCalculated: " << calculated << "\nReference:  " << reference << "\nx = " << x;

        measuredMaxAbsError = math::Max(measuredMaxAbsError, absError);
        measuredMaxRelError = math::Max(measuredMaxRelError, relError);

        if (range.type == TestRange::StepType::Increment)
            x += range.step;
        else
            x *= range.step;
    }

    RT_LOG_INFO("math::%s (float) max absolute error = %.3e, max relative error: %.3e",
                name, measuredMaxAbsError, measuredMaxRelError);
}

} // namespace

//////////////////////////////////////////////////////////////////////////

TEST(MathTest, Sin)
{
    const auto func = [](Float x)
    {
        return math::Sin(x);
    };

    TestRange range(-10.0f, 10.0f, 0.01f, TestRange::StepType::Increment);
    TestTranscendental("Sin", range, func, sinf, 1.0e-06f, 1.0f);
}

TEST(MathTest, Sin_4)
{
    const auto func = [](Float x)
    {
        return math::Sin(math::Vector4(x)).x;
    };

    TestRange range(-10.0f, 10.0f, 0.01f, TestRange::StepType::Increment);
    TestTranscendental("Sin_4", range, func, sinf, 2.0e-06f, 1.0f);
}

TEST(MathTest, Sin_8)
{
    const auto func = [](Float x)
    {
        return math::Sin(math::Vector8(x))[0];
    };

    TestRange range(-10.0f, 10.0f, 0.01f, TestRange::StepType::Increment);
    TestTranscendental("Sin_8", range, func, sinf, 2.0e-06f, 1.0f);
}

TEST(MathTest, Cos)
{
    TestRange range(-10.0f, 10.0f, 0.01f, TestRange::StepType::Increment);
    TestTranscendental("Cos", range, math::Cos, cosf, 1.0e-06f, 1.0f);
}

TEST(MathTest, FastACos)
{
    TestRange range(-1.0f, 1.0f, 0.01f, TestRange::StepType::Increment);
    TestTranscendental("ACos", range, math::FastACos, acosf, 7.0e-5f, 1.0f);
}

TEST(MathTest, FastExp)
{
    TestRange range(-5.0f, 5.0f, 0.01f, TestRange::StepType::Increment);
    TestTranscendental("FastExp", range, math::FastExp, expf, 1.0f, 2.0e-2f);
}

TEST(MathTest, Log)
{
    TestRange range(0.0001f, 1.0e+30f, 1.5f, TestRange::StepType::Multiply);
    TestTranscendental("Log", range, math::Log, logf, 1.0f, 3.0e-07f);
}

TEST(MathTest, FastLog)
{
    TestRange range(0.0001f, 1.0e+30f, 1.5f, TestRange::StepType::Multiply);
    TestTranscendental("FastLog", range, math::FastLog, logf, 1.0f, 1.0e-4f);
}

// TODO atan2