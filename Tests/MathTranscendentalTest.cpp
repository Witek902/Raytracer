#include "PCH.h"
#include "../Core/Math/Transcendental.h"
#include "../Core/Math/Math.h"
#include "../Core/Utils/Logger.h"

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
    const auto func = [](float x) { return math::Sin(x); };
    const TestRange range(-10.0f, 10.0f, 0.01f, TestRange::StepType::Increment);
    TestTranscendental("Sin", range, func, sinf, 1.0e-06f, 1.0f);
}

TEST(MathTest, Sin_4)
{
    const auto func = [](float x) { return math::Sin(math::Vector4(x)).x; };
    const TestRange range(-10.0f, 10.0f, 0.01f, TestRange::StepType::Increment);
    TestTranscendental("Sin_4", range, func, sinf, 2.0e-06f, 1.0f);
}

TEST(MathTest, Sin_8)
{
    const auto func = [](float x) { return math::Sin(math::Vector8(x))[0]; };
    const TestRange range(-10.0f, 10.0f, 0.01f, TestRange::StepType::Increment);
    TestTranscendental("Sin_8", range, func, sinf, 2.0e-06f, 1.0f);
}

TEST(MathTest, Cos)
{
    const auto func = [](float x) { return math::Cos(x); };
    const TestRange range(-10.0f, 10.0f, 0.01f, TestRange::StepType::Increment);
    TestTranscendental("Cos", range, func, cosf, 1.0e-06f, 1.0f);
}

TEST(MathTest, FastACos)
{
    const TestRange range(-1.0f, 1.0f, 0.01f, TestRange::StepType::Increment);
    TestTranscendental("ACos", range, math::FastACos, acosf, 7.0e-5f, 1.0f);
}

TEST(MathTest, FastExp)
{
    const auto func = [](float x) { return math::FastExp(x); };
    const TestRange range(-40.0f, 5.0f, 0.01f, TestRange::StepType::Increment);
    TestTranscendental("FastExp", range, func, expf, 1.0f, 2.0e-2f);
}

TEST(MathTest, FastExp_4)
{
    const auto func = [](float x) { return math::FastExp(math::Vector4(x)).x; };
    const TestRange range(-40.0f, 5.0f, 0.01f, TestRange::StepType::Increment);
    TestTranscendental("FastExp_4", range, func, expf, 1.0f, 2.0e-2f);
}

TEST(MathTest, Log)
{
    const TestRange range(0.0001f, 1.0e+30f, 1.5f, TestRange::StepType::Multiply);
    TestTranscendental("Log", range, math::Log, logf, 1.0f, 3.0e-07f);
}

TEST(MathTest, FastLog)
{
    const auto func = [](float x) { return math::FastLog(x); };
    TestRange range(0.0001f, 1.0e+30f, 1.5f, TestRange::StepType::Multiply);
    TestTranscendental("FastLog", range, func, logf, 1.0f, 1.0e-4f);
}

TEST(MathTest, FastLog_4)
{
    const auto func = [](float x) { return math::FastLog(math::Vector4(x)).x; };
    TestRange range(0.0001f, 1.0e+30f, 1.5f, TestRange::StepType::Multiply);
    TestTranscendental("FastLog_4", range, func, logf, 1.0f, 1.0e-4f);
}

// TODO atan2