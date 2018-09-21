#pragma once

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // _DEBUG

// SSE
#include <xmmintrin.h>
#include <smmintrin.h>
#include <immintrin.h>
#include <emmintrin.h>
#include <intrin.h>

#include <inttypes.h>
#include <stddef.h>
#include <assert.h>
#include <math.h>
#include <vector>
#include <memory>
#include <initializer_list>
#include <functional>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <sstream>
#include <algorithm>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include "Utils/iacaMarks.h"