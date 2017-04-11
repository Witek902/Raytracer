#pragma once

#include <inttypes.h>
#include <stddef.h>

#define UNUSED(x) (void)(x)

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