#pragma once

#if defined(_DEBUG) && defined(WIN32)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // _DEBUG

// SSE
#include <xmmintrin.h>
#include <smmintrin.h>
#include <immintrin.h>
#include <emmintrin.h>

#include <cassert>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>
#include <math.h>
#include <float.h>
#include <stdarg.h>
#include <memory>
#include <initializer_list>
#include <functional>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <sstream>
#include <algorithm>

#ifdef WIN32
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <Windows.h>
#elif defined(__linux__) | defined(__LINUX__)
#else
    #error "Target platform not supported."
#endif // WIN32

#include "Utils/iacaMarks.h"