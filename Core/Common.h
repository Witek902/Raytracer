#pragma once

#include <inttypes.h>
#include <stddef.h>
#include <float.h>

// disable some Visual Studio specific warnings
#ifdef _MSC_VER

// "needs to have dll-interface to be used by clients of class ..."
#pragma warning (disable : 4251)

// "C++ nonstandard extension: nameless struct"
#pragma warning(disable : 4201)

// "structure was padded due to alignment specifier"
#pragma warning(disable : 4324)

// "conditional expression is constant"
#pragma warning(disable : 4127)

#endif // _MSC_VER


// TODO assertions should be disabled in "Final" build
#define RT_ENABLE_ASSERTS

// TODO #define RT_USE_SSE

#define RT_USE_AVX
#define RT_USE_FMA
#define RT_USE_AVX2
#define RT_USE_FP16C


#define RT_UNUSED(x) (void)(x)
#define RT_INLINE inline

// macro forcing a function to be inlined
#if defined(__LINUX__) | defined(__linux__)
#define RT_FORCE_INLINE inline __attribute__((always_inline))
#elif defined(WIN32)
#define RT_FORCE_INLINE __forceinline
#endif // defined(__LINUX__) | defined(__linux__)

// macro forcing a function not to be inlined
#ifdef WIN32
#define RT_FORCE_NOINLINE __declspec(noinline)
#elif defined(__LINUX__) || defined(__linux__)
#define RT_FORCE_NOINLINE __attribute__((noinline))
#endif // WIN32

// aligning macro for objects using SIMD registers
#if defined(WIN32)
#define RT_ALIGN(bytes) alignas(bytes)
#elif defined(__LINUX__) | defined(__linux__)
#define RT_ALIGN(bytes) __attribute__((aligned(bytes)))
#else
#error "Target system not supported!"
#endif // defined(WIN32)

#define RT_CACHE_LINE_SIZE 64u

// force global variable definition to be shared across all compilation units
#if defined(WIN32)
#define RT_GLOBAL_CONST extern const __declspec(selectany)
#elif defined(__LINUX__) | defined(__linux__)
#define RT_GLOBAL_CONST const
#else
#error "Target system not supported!"
#endif // defined(WIN32)

#define RT_PREFETCH_L1(addr) _mm_prefetch((const char*)(addr), _MM_HINT_T0);
#define RT_PREFETCH_L2(addr) _mm_prefetch((const char*)(addr), _MM_HINT_T1);
#define RT_PREFETCH_L3(addr) _mm_prefetch((const char*)(addr), _MM_HINT_T2);

// fatal error
#if defined(WIN32)
#define RT_FATAL(...) __debugbreak()
#elif defined(__LINUX__) | defined(__linux__)
#define RT_FATAL(...) __builtin_trap()
#else
#error "Target system not supported!"
#endif // defined(WIN32)

// debug break
#if defined(WIN32)
#define RT_BREAK() __debugbreak()
#elif defined(__LINUX__) | defined(__linux__)
#define RT_BREAK() __builtin_trap()
#else
#error "Target system not supported!"
#endif // defined(WIN32)


#ifdef RT_ENABLE_ASSERTS
#define RT_ASSERT(expression, ...) \
do { \
    if (!(expression)) \
    { \
        RT_FATAL(__VA_ARGS__); \
    } \
} while (0)
#else
#define RT_ASSERT(expression, ...)
#endif // RT_ENABLE_ASSERTS


#if defined(__LINUX__) | defined(__linux__)
using Uint64 = uint64_t;
using Int64 = int64_t;
#elif defined(_WINDOWS)
using Uint64 = unsigned __int64;
using Int64 = __int64;
#endif // defined(__LINUX__) | defined(__linux__)

using Uint32 = unsigned int;
using Int32 = signed int;
using Uint16 = unsigned short;
using Int16 = signed short;
using Uint8 = unsigned char;
using Int8 = signed char;

class NoCopyable
{
public:
    NoCopyable() = default;
    NoCopyable(NoCopyable&&) = default;
    NoCopyable& operator = (NoCopyable&&) = default;
private:
    NoCopyable(const NoCopyable&) = delete;
    NoCopyable& operator = (const NoCopyable&) = delete;
};