#pragma once

#if defined(_DEBUG) && defined(WIN32)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // _DEBUG

#ifdef WIN32
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <Windows.h>
    #include <windowsx.h>
    #include <strsafe.h>
#elif defined(__linux__) | defined(__LINUX__)
#else
    #error "Target platform not supported."
#endif // WIN32

#include <sstream>
#include <string>
#include <vector>
#include <memory>

#include "../External/cxxopts.hpp"
#include "../External/tiny_obj_loader.h"
#include "../External/imgui/imgui.h"