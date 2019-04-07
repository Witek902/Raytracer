#include "PCH.h"
#include "../Core/Utils/HashGrid.h"

#include <benchmark/benchmark.h>

using namespace rt;
using namespace math;

static void Benchmark_HashGrid_Collect(benchmark::State& state)
{
    const Uint32 numPoints = 1000000;
    const float particleRadius = 0.4f;
    const float boxSize = 100.0f;

    Random random;

    struct Particle
    {
        Vector4 pos;
        RT_FORCE_INLINE const Vector4& GetPosition() const { return pos; }
    };

    DynArray<Particle> particles;
    for (Uint32 i = 0; i < numPoints; ++i)
    {
        particles.PushBack({ random.GetVector4() * boxSize });
    }

    HashGrid grid;
    grid.Build(particles, particleRadius);
    const Box& box = grid.GetBox();

    struct Query
    {
        void operator()(Uint32 index) { dummy += index; }
        Uint32 dummy = 0;
    };

    Query query;
    for (auto _ : state)
    {
        const Vector4 queryPoint = random.GetVector4() * (box.max - box.min) + box.min;
        grid.Process(queryPoint, particles, query);
    }

    printf("%u\n", query.dummy);
}
BENCHMARK(Benchmark_HashGrid_Collect);
