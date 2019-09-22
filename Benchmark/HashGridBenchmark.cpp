#include "PCH.h"
#include "../Core/Utils/HashGrid.h"

#include <benchmark/benchmark.h>

using namespace rt;
using namespace math;

static void Benchmark_HashGrid_Collect(benchmark::State& state)
{
    const uint32 numPoints = 1000000;
    const float particleRadius = 0.4f;
    const float boxSize = 100.0f;

    Random random;

    struct Particle
    {
        Vector4 pos;
        RT_FORCE_INLINE const Vector4& GetPosition() const { return pos; }
    };

    DynArray<Particle> particles;
    for (uint32 i = 0; i < numPoints; ++i)
    {
        particles.PushBack({ random.GetVector4() * boxSize });
    }

    HashGrid grid;
    grid.Build(particles, particleRadius);
    const Box& box = grid.GetBox();

    struct Query
    {
        void operator()(uint32 index) { dummy += index; }
        uint32 dummy = 0;
    };

    Query query;
    for (auto _ : state)
    {
        const Vector4 queryPoint = random.GetVector4() * (box.max - box.min) + box.min;
        grid.Process(queryPoint, particles, query);
    }

    benchmark::DoNotOptimize(query);
}
BENCHMARK(Benchmark_HashGrid_Collect);
