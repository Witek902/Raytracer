#include "PCH.h"
#include "../RaytracerLib/Math/Transcendental.h"

#include <benchmark/benchmark.h>

using namespace rt;
using namespace math;


static void Benchmark_Transcendental_Sin(benchmark::State& state)
{
    Float x = 0.0f;
    Float y = 0.0f;
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
    Vector4 y;
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
    Float x = 0.0f;
    Float y = 0.0f;
    for (auto _ : state)
    {
        y += FastACos(x);
        x += 1.0e-5f;
    }
    benchmark::DoNotOptimize(y);
}
BENCHMARK(Benchmark_Transcendental_FastACos);