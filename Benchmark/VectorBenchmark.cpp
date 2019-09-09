#include "PCH.h"
#include "../Core/Math/Vector4.h"
#include "../Core/Math/Random.h"

#include <benchmark/benchmark.h>

using namespace rt;
using namespace math;

static void Benchmark_Vector4_Normalize1(benchmark::State& state)
{
    Random random;
    Vector4 vec = Vector4(1.0f, 1.0f, 1.0f, 0.0f) + random.GetVector4Bipolar() * 0.5f;

    for (auto _ : state)
    {
        vec.Normalize3();
    }

    benchmark::DoNotOptimize(vec);
}
BENCHMARK(Benchmark_Vector4_Normalize1);

static void Benchmark_Vector4_Normalize4(benchmark::State& state)
{
    Random random;
    Vector4 vecA = Vector4(1.0f, 1.0f, 1.0f, 0.0f) + random.GetVector4Bipolar() * 0.5f;
    Vector4 vecB = Vector4(1.0f, 1.0f, 1.0f, 0.0f) + random.GetVector4Bipolar() * 0.5f;
    Vector4 vecC = Vector4(1.0f, 1.0f, 1.0f, 0.0f) + random.GetVector4Bipolar() * 0.5f;
    Vector4 vecD = Vector4(1.0f, 1.0f, 1.0f, 0.0f) + random.GetVector4Bipolar() * 0.5f;

    for (auto _ : state)
    {
        vecA.Normalize3();
        vecB.Normalize3();
        vecC.Normalize3();
        vecD.Normalize3();
    }

    benchmark::DoNotOptimize(vecA);
    benchmark::DoNotOptimize(vecB);
    benchmark::DoNotOptimize(vecC);
    benchmark::DoNotOptimize(vecD);
}
BENCHMARK(Benchmark_Vector4_Normalize4);

static void Benchmark_Vector4_FastNormalize1(benchmark::State& state)
{
    Random random;
    Vector4 vec = Vector4(1.0f, 1.0f, 1.0f, 0.0f) + random.GetVector4Bipolar() * 0.5f;

    for (auto _ : state)
    {
        vec.FastNormalize3();
    }

    benchmark::DoNotOptimize(vec);
}
BENCHMARK(Benchmark_Vector4_FastNormalize1);

static void Benchmark_Vector4_FastNormalize4(benchmark::State& state)
{
    Random random;
    Vector4 vecA = Vector4(1.0f, 1.0f, 1.0f, 0.0f) + random.GetVector4Bipolar() * 0.5f;
    Vector4 vecB = Vector4(1.0f, 1.0f, 1.0f, 0.0f) + random.GetVector4Bipolar() * 0.5f;
    Vector4 vecC = Vector4(1.0f, 1.0f, 1.0f, 0.0f) + random.GetVector4Bipolar() * 0.5f;
    Vector4 vecD = Vector4(1.0f, 1.0f, 1.0f, 0.0f) + random.GetVector4Bipolar() * 0.5f;

    for (auto _ : state)
    {
        vecA.FastNormalize3();
        vecB.FastNormalize3();
        vecC.FastNormalize3();
        vecD.FastNormalize3();
    }

    benchmark::DoNotOptimize(vecA);
    benchmark::DoNotOptimize(vecB);
    benchmark::DoNotOptimize(vecC);
    benchmark::DoNotOptimize(vecD);
}
BENCHMARK(Benchmark_Vector4_FastNormalize4);
