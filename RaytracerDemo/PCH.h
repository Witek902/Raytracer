#pragma once

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // _DEBUG

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <windowsx.h>

#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <strsafe.h>

#include "../External/cxxopts.hpp"
#include "../External/tiny_obj_loader.h"
#include "../External/imgui/imgui.h"