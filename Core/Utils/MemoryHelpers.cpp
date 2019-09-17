#include "PCH.h"
#include "MemoryHelpers.h"

#include <intrin.h>

namespace rt {

void LargeMemCopy(void* __restrict dest, const void* __restrict src, size_t size)
{
#ifdef RT_USE_AVX2

    constexpr size_t alingment = 32;

    RT_ASSERT(((size_t)dest + size >= (size_t)src) || ((size_t)src + size >= (size_t)dest), "Source and destination pointers cannot overlap");
    RT_ASSERT((size_t)dest % alingment == 0, "Destination pointer must be aligned");
    RT_ASSERT((size_t)src % alingment == 0, "Source pointer must be aligned");
    RT_ASSERT((size_t)size % alingment == 0, "Data size must be aligned");

    const size_t stride = 8u;
    size_t sizeInElements = size >> 5u;

    const __m256i* __restrict simdSrc = reinterpret_cast<const __m256i * __restrict>(src);
    __m256i* __restrict simdDest = reinterpret_cast<__m256i * __restrict>(dest);

    while (sizeInElements >= stride)
    {
        const __m256i r0 = _mm256_load_si256(simdSrc++);
        const __m256i r1 = _mm256_load_si256(simdSrc++);
        const __m256i r2 = _mm256_load_si256(simdSrc++);
        const __m256i r3 = _mm256_load_si256(simdSrc++);
        const __m256i r4 = _mm256_load_si256(simdSrc++);
        const __m256i r5 = _mm256_load_si256(simdSrc++);
        const __m256i r6 = _mm256_load_si256(simdSrc++);
        const __m256i r7 = _mm256_load_si256(simdSrc++);

#ifdef _MSC_VER
        _ReadWriteBarrier();
#endif

        _mm256_stream_si256(simdDest++, r0);
        _mm256_stream_si256(simdDest++, r1);
        _mm256_stream_si256(simdDest++, r2);
        _mm256_stream_si256(simdDest++, r3);
        _mm256_stream_si256(simdDest++, r4);
        _mm256_stream_si256(simdDest++, r5);
        _mm256_stream_si256(simdDest++, r6);
        _mm256_stream_si256(simdDest++, r7);

        sizeInElements -= stride;
    }

    // flush non-temporal stores
    _mm_sfence();

    for (size_t i = 0; i < sizeInElements; ++i)
    {
        _mm256_store_si256(simdDest + i, _mm256_load_si256(simdSrc + i));
    }

#elif defined(RT_USE_SSE)

    constexpr size_t alingment = 16;

    RT_ASSERT(((size_t)dest + size >= (size_t)src) || ((size_t)src + size >= (size_t)dest), "Source and destination pointers cannot overlap");
    RT_ASSERT((size_t)dest % alingment == 0, "Destination pointer must be aligned");
    RT_ASSERT((size_t)src % alingment == 0, "Source pointer must be aligned");
    RT_ASSERT((size_t)size % alingment == 0, "Data size must be aligned");

    const size_t stride = 8u;
    size_t sizeInElements = size >> 4u;

    const __m128i* __restrict simdSrc = reinterpret_cast<const __m128i * __restrict>(src);
    __m128i* __restrict simdDest = reinterpret_cast<__m128i * __restrict>(dest);

    while (sizeInElements >= stride)
    {
        const __m128i r0 = _mm_load_si128(simdSrc++);
        const __m128i r1 = _mm_load_si128(simdSrc++);
        const __m128i r2 = _mm_load_si128(simdSrc++);
        const __m128i r3 = _mm_load_si128(simdSrc++);
        const __m128i r4 = _mm_load_si128(simdSrc++);
        const __m128i r5 = _mm_load_si128(simdSrc++);
        const __m128i r6 = _mm_load_si128(simdSrc++);
        const __m128i r7 = _mm_load_si128(simdSrc++);

#ifdef _MSC_VER
        _ReadWriteBarrier();
#endif

        _mm_stream_si128(simdDest++, r0);
        _mm_stream_si128(simdDest++, r1);
        _mm_stream_si128(simdDest++, r2);
        _mm_stream_si128(simdDest++, r3);
        _mm_stream_si128(simdDest++, r4);
        _mm_stream_si128(simdDest++, r5);
        _mm_stream_si128(simdDest++, r6);
        _mm_stream_si128(simdDest++, r7);

        sizeInElements -= stride;
    }

    // flush non-temporal stores
    _mm_sfence();

    for (size_t i = 0; i < sizeInElements; ++i)
    {
        _mm_store_si128(simdDest + i, _mm_load_si128(simdSrc + i));
    }

#else

    memcpy(dest, src, size);

#endif // RT_USE_AVX2
}

} // rt
