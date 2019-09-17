#include "PCH.h"
#include "../Core/Utils/Memory.h"
#include "../Core/Utils/MemoryHelpers.h"

#include <benchmark/benchmark.h>

using namespace rt;


static void Benchmark_LargeMemCopy_Aligned64(benchmark::State& state)
{
    const size_t dataSize = static_cast<size_t>(state.range(0) * 1024 * 1024);
    void* src = DefaultAllocator::Allocate(dataSize, RT_CACHE_LINE_SIZE);
    void* dest = DefaultAllocator::Allocate(dataSize, RT_CACHE_LINE_SIZE);

    for (auto _ : state)
    {
        LargeMemCopy(dest, src, dataSize);
    }

    DefaultAllocator::Free(dest);
    DefaultAllocator::Free(src);
}
BENCHMARK(Benchmark_LargeMemCopy_Aligned64)->RangeMultiplier(2)->Range(1, 128);


static void Benchmark_Memcpy_Std(benchmark::State& state)
{
    const size_t dataSize = static_cast<size_t>(state.range(0) * 1024 * 1024);
    void* src = DefaultAllocator::Allocate(dataSize, RT_CACHE_LINE_SIZE);
    void* dest = DefaultAllocator::Allocate(dataSize, RT_CACHE_LINE_SIZE);

    for (auto _ : state)
    {
        memcpy(dest, src, dataSize);
    }

    DefaultAllocator::Free(dest);
    DefaultAllocator::Free(src);
}
BENCHMARK(Benchmark_Memcpy_Std)->RangeMultiplier(2)->Range(1, 128);
