#include "PCH.h"
#include "../Core/Math/Random.h"

#include <benchmark/benchmark.h>

using namespace rt;
using namespace math;


static void Benchmark_Random_Int(benchmark::State& state)
{
    Random random;
    for (auto _ : state)
    {
        random.GetInt();
    }
    benchmark::DoNotOptimize(random.GetInt());
}
BENCHMARK(Benchmark_Random_Int);


static void Benchmark_Random_Float(benchmark::State& state)
{
    Random random;
    for (auto _ : state)
    {
        random.GetFloat();
    }
    benchmark::DoNotOptimize(random.GetFloat());
}
BENCHMARK(Benchmark_Random_Float);


static void Benchmark_Random_Float2_Normal(benchmark::State& state)
{
    Random random;
    for (auto _ : state)
    {
        random.GetFloatNormal2();
    }
    benchmark::DoNotOptimize(random.GetFloatNormal2());
}
BENCHMARK(Benchmark_Random_Float2_Normal);


static void Benchmark_Random_Vector4(benchmark::State& state)
{
    Random random;
    for (auto _ : state)
    {
        random.GetVector4();
    }
    benchmark::DoNotOptimize(random.GetVector4());
}
BENCHMARK(Benchmark_Random_Vector4);


static void Benchmark_Random_Vector8(benchmark::State& state)
{
    Random random;
    for (auto _ : state)
    {
        random.GetVector8();
    }
    benchmark::DoNotOptimize(random.GetVector8());
}
BENCHMARK(Benchmark_Random_Vector8);


static void Benchmark_Random_Circle(benchmark::State& state)
{
    Random random;
    for (auto _ : state)
    {
        random.GetCircle();
    }
    benchmark::DoNotOptimize(random.GetCircle());
}
BENCHMARK(Benchmark_Random_Circle);


static void Benchmark_Random_Circle8(benchmark::State& state)
{
    Random random;
    for (auto _ : state)
    {
        random.GetCircle_Simd8();
    }
    benchmark::DoNotOptimize(random.GetCircle_Simd8());
}
BENCHMARK(Benchmark_Random_Circle8);


static void Benchmark_Random_Triangle(benchmark::State& state)
{
    Random random;
    for (auto _ : state)
    {
        random.GetTriangle();
    }
    benchmark::DoNotOptimize(random.GetTriangle());
}
BENCHMARK(Benchmark_Random_Triangle);


static void Benchmark_Random_Hexagon(benchmark::State& state)
{
    Random random;
    for (auto _ : state)
    {
        random.GetHexagon();
    }
    benchmark::DoNotOptimize(random.GetHexagon());
}
BENCHMARK(Benchmark_Random_Hexagon);


static void Benchmark_Random_Sphere(benchmark::State& state)
{
    Random random;
    for (auto _ : state)
    {
        random.GetSphere();
    }
    benchmark::DoNotOptimize(random.GetSphere());
}
BENCHMARK(Benchmark_Random_Sphere);


static void Benchmark_Random_HemishpereCos(benchmark::State& state)
{
    Random random;
    for (auto _ : state)
    {
        random.GetHemishpereCos();
    }
    benchmark::DoNotOptimize(random.GetHemishpereCos());
}
BENCHMARK(Benchmark_Random_HemishpereCos);