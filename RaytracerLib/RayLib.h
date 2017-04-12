#pragma once

// "needs to have dll-interface to be used by clients of class ..."
#pragma warning (disable : 4251)

#include "Common.h"

// DLL import / export macro
#ifdef RAYLIB_EXPORTS
#define RAYLIB_API __declspec(dllexport)
#else // RAYLIB_EXPORTS
#define RAYLIB_API __declspec(dllimport)
#endif // RAYLIB_EXPORTS
