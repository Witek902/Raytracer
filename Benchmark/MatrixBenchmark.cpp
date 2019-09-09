#include "PCH.h"
#include "../Core/Math/Matrix4.h"

#include <benchmark/benchmark.h>

using namespace rt;
using namespace math;

static void Benchmark_Matrix4_Multiply(benchmark::State& state)
{
    const Matrix4 matA
    {
        Vector4(1.0f, 0.0f, 0.0f, 0.0f),
        Vector4(0.0f, 1.0f, 0.0f, 0.0f),
        Vector4(0.0f, 0.0f, 1.0f, 0.0f),
        Vector4(0.0f, 0.0f, 0.0f, 1.0f)
    };

    const Matrix4 matB
    {
        Vector4(0.0f, 0.0f, 0.0f, 1.0f),
        Vector4(0.0f, 1.0f, 0.0f, 0.0f),
        Vector4(0.0f, 0.0f, 1.0f, 0.0f),
        Vector4(1.0f, 0.0f, 0.0f, 0.0f)
    };

    const Matrix4 matC
    {
        Vector4(0.0f, 0.0f, 1.0f, 0.0f),
        Vector4(1.0f, 0.0f, 0.0f, 0.0f),
        Vector4(0.0f, 0.0f, 0.0f, 1.0f),
        Vector4(0.0f, 1.0f, 0.0f, 0.0f)
    };

    Matrix4 mat = matA;

    for (auto _ : state)
    {
        mat = mat * matA;
        mat = mat * matB;
        mat = mat * matC;
    }

    benchmark::DoNotOptimize(mat);
}
BENCHMARK(Benchmark_Matrix4_Multiply);
