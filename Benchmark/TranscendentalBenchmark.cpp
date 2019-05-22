#include "PCH.h"
#include "../Core/Math/Transcendental.h"

#include <benchmark/benchmark.h>

using namespace rt;
using namespace math;


static void Benchmark_Transcendental_Sin(benchmark::State& state)
{
    float x = 0.0f;
    float y = 0.0f;
    for (auto _ : state)
    {
        y += Sin(x);
        x += 1.0e-5f;
    }
    benchmark::DoNotOptimize(y);
}
BENCHMARK(Benchmark_Transcendental_Sin);

static void Benchmark_Transcendental_Sin4(benchmark::State& state)
{
    Vector4 x(0.0f, 1.0e-6f, 2.0e-6f, 3.0e-6f);
    Vector4 y = Vector4::Zero();
    for (auto _ : state)
    {
        y += Sin(x);
        x += Vector4(1.0e-5f);
    }
    benchmark::DoNotOptimize(y);
}
BENCHMARK(Benchmark_Transcendental_Sin4);


static void Benchmark_Transcendental_FastACos(benchmark::State& state)
{
    float x = 0.0f;
    float y = 0.0f;
    for (auto _ : state)
    {
        y += FastACos(x);
        x += 1.0e-5f;
    }
    benchmark::DoNotOptimize(y);
}
BENCHMARK(Benchmark_Transcendental_FastACos);


static void Benchmark_Transcendental_FastExp(benchmark::State& state)
{
    float x = -80.0f;
    float y = 0.0f;
    for (auto _ : state)
    {
        y += FastExp(x);
        x += 1.0e-6f;
    }
    benchmark::DoNotOptimize(y);
}
BENCHMARK(Benchmark_Transcendental_FastExp);

static void Benchmark_Transcendental_FastExp4(benchmark::State& state)
{
    Vector4 x(-80.0f, -80.1f, -80.2f, -80.3f);
    Vector4 y = Vector4::Zero();
    for (auto _ : state)
    {
        y += FastExp(x);
        x += Vector4(1.0e-6f);
    }
    benchmark::DoNotOptimize(y);
}
BENCHMARK(Benchmark_Transcendental_FastExp4);


static void Benchmark_Transcendental_Log(benchmark::State& state)
{
    float x = 1.0e-10f;
    float y = 0.0f;
    for (auto _ : state)
    {
        y += Log(x);
        x += 1.0e-3f;
    }
    benchmark::DoNotOptimize(y);
}
BENCHMARK(Benchmark_Transcendental_Log);


static void Benchmark_Transcendental_FastLog(benchmark::State& state)
{
    float x = 1.0e-10f;
    float y = 0.0f;
    for (auto _ : state)
    {
        y += FastLog(x);
        x += 1.0e-3f;
    }
    benchmark::DoNotOptimize(y);
}
BENCHMARK(Benchmark_Transcendental_FastLog);

static void Benchmark_Transcendental_FastLog4(benchmark::State& state)
{
    Vector4 x(1.0e-10f, 1.1e-10f, 1.2e-10f, 1.3e-10f);
    Vector4 y = Vector4::Zero();
    for (auto _ : state)
    {
        y += FastLog(x);
        x += Vector4(1.0e-3f);
    }
    benchmark::DoNotOptimize(y);
}
BENCHMARK(Benchmark_Transcendental_FastLog4);