#pragma once

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // _DEBUG

// SSE
#include <xmmintrin.h>
#include <smmintrin.h>

#include <vector>
#include <memory>
#include <initializer_list>