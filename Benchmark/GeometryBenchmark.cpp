#include "PCH.h"
#include "../Core/Math/Geometry.h"
#include "../Core/Math/Random.h"

#include <benchmark/benchmark.h>

using namespace rt;
using namespace math;

static void Benchmark_Geometry_BuildOrthonormalBasis(benchmark::State& state)
{
    Random random;

    Vector4 x = random.GetSphere();

    for (auto _ : state)
    {
        Vector4 u, v;
        BuildOrthonormalBasis(x, u, v);
        x = (u + v).Normalized3();
    }
    benchmark::DoNotOptimize(x);

    printf("%f", x.x);
}
BENCHMARK(Benchmark_Geometry_BuildOrthonormalBasis);


static void Benchmark_Geometry_RayTriIntersection(benchmark::State& state)
{
    Random random;

    const Uint32 numRays = 1024 * 16;
    std::vector<Ray> rays;

    const Uint32 numVectors = 1024 * 16;
    std::vector<Vector4> v0;
    std::vector<Vector4> v1;
    std::vector<Vector4> v2;

    for (Uint32 i = 0; i < numRays; ++i)
    {
        rays.push_back(Ray(random.GetVector4(), random.GetVector4()));
    }

    for (Uint32 i = 0; i < numVectors; ++i)
    {
        v0.push_back(random.GetVector4());
        v1.push_back(random.GetVector4());
        v2.push_back(random.GetVector4());
    }

    Uint32 i = 0;
    Float tmin = FLT_MAX;
    for (auto _ : state)
    {
        Uint32 rayIndex = i % numRays;
        Uint32 triIndex = (i / numRays) % numVectors;

        float u, v, t;
        if (Intersect_TriangleRay(rays[rayIndex], v0[triIndex], v1[triIndex], v2[triIndex], u, v, t))
        {
            tmin = Min(tmin, t);
        }

        i++;
    }
    benchmark::DoNotOptimize(tmin);

    printf("%f", tmin);
}
BENCHMARK(Benchmark_Geometry_RayTriIntersection);
