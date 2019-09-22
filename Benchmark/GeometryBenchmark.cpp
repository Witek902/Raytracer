#include "PCH.h"
#include "../Core/Math/Geometry.h"
#include "../Core/Math/Random.h"
#include "../Core/Math/SamplingHelpers.h"

#include <benchmark/benchmark.h>

using namespace rt;
using namespace math;

static void Benchmark_Geometry_BuildOrthonormalBasis(benchmark::State& state)
{
    Random random;

    Vector4 x = SamplingHelpers::GetSphere(random.GetFloat2());

    for (auto _ : state)
    {
        Vector4 u, v;
        BuildOrthonormalBasis(x, u, v);
        x = (u + v).Normalized3();
    }
    benchmark::DoNotOptimize(x);
}
BENCHMARK(Benchmark_Geometry_BuildOrthonormalBasis);


static void Benchmark_Geometry_RayTriIntersection(benchmark::State& state)
{
    Random random;

    const uint32 numRays = 1024 * 16;
    std::vector<Ray> rays;

    const uint32 numVectors = 1024 * 16;
    std::vector<Vector4> v0;
    std::vector<Vector4> v1;
    std::vector<Vector4> v2;

    for (uint32 i = 0; i < numRays; ++i)
    {
        rays.push_back(Ray(random.GetVector4(), random.GetVector4()));
    }

    for (uint32 i = 0; i < numVectors; ++i)
    {
        v0.push_back(random.GetVector4());
        v1.push_back(random.GetVector4());
        v2.push_back(random.GetVector4());
    }

    uint32 i = 0;
    float tmin = FLT_MAX;
    for (auto _ : state)
    {
        uint32 rayIndex = i % numRays;
        uint32 triIndex = (i / numRays) % numVectors;

        float u, v, t;
        if (Intersect_TriangleRay(rays[rayIndex], v0[triIndex], v1[triIndex], v2[triIndex], u, v, t))
        {
            tmin = Min(tmin, t);
        }

        i++;
    }
    benchmark::DoNotOptimize(tmin);
}
BENCHMARK(Benchmark_Geometry_RayTriIntersection);
