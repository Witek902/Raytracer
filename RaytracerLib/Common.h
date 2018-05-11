#pragma once

#include <inttypes.h>
#include <stddef.h>


// C++ nonstandard extension: nameless struct
#pragma warning(disable : 4201)


#define RT_UNUSED(x) (void)(x)
#define RT_INLINE inline
#define RT_FORCE_INLINE __forceinline
#define RT_FORCE_NOINLINE __declspec(noinline)
#define RT_ALIGN(x) __declspec(align(x))

#define RT_CACHE_LINE_SIZE 64

#define RT_PREFETCH_L1(addr) _mm_prefetch((const char*)(addr), _MM_HINT_T0);
#define RT_PREFETCH_L2(addr) _mm_prefetch((const char*)(addr), _MM_HINT_T1);

#if defined(__LINUX__) | defined(__linux__)
typedef uint64_t Uint64;
typedef int64_t Int64;
#elif defined(_WINDOWS)
typedef unsigned __int64 Uint64;
typedef __int64 Int64;
#endif // defined(__LINUX__) | defined(__linux__)

typedef unsigned int Uint32;
typedef signed int Int32;
typedef unsigned short Uint16;
typedef signed short Int16;
typedef unsigned char Uint8;
typedef signed char Int8;

typedef bool Bool;
typedef float Float;
typedef double Double;



namespace rt {

union Bits32
{
    Float f;
    Uint32 ui;
    Int32 si;
};

union Bits64
{
    Double f;
    Uint64 ui;
    Int64 si;
};

} // namespace rt
