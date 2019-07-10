#include "PCH.h"
#include "../Core/Math/Packed.h"
#include "../Core/Math/Random.h"
#include "../Core/Math/SamplingHelpers.h"

#include <benchmark/benchmark.h>

using namespace rt;
using namespace math;

static void Benchmark_Packed_UnitVector(benchmark::State& state)
{
    Random random;
    Vector4 x = SamplingHelpers::GetSphere(random.GetFloat2()).Normalized3();

    for (auto _ : state)
    {
        PackedUnitVector3 packed;
        packed.FromVector(x);
        x = packed.ToVector();
    }
    benchmark::DoNotOptimize(x);

    printf("%f", x.x);
}
BENCHMARK(Benchmark_Packed_UnitVector);


static void Benchmark_Packed_ColorRgbHdr(benchmark::State& state)
{
    Random random;
    Vector4 x = random.GetVector4();

    for (auto _ : state)
    {
        PackedColorRgbHdr packed;
        packed.FromVector(x);
        x = packed.ToVector();
    }
    benchmark::DoNotOptimize(x);

    printf("%f", x.x);
}
BENCHMARK(Benchmark_Packed_ColorRgbHdr);
