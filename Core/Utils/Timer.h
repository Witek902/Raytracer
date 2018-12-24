#pragma once

#include "../RayLib.h"

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#elif defined(__LINUX__) | defined(__linux__)
#include <time.h>
#endif


/**
 * High resolution timer.
 */
class RAYLIB_API Timer
{
public:
    Timer();

    /**
     * Start time measurement
     */
    void Start();

    /**
     * Stop time measurement
     * @return Seconds elapsed since last Start() call
     */
    double Stop();

    /**
     * Reset and return current time.
     */
    double Reset();

private:

#if defined(WIN32)
    LARGE_INTEGER mStart; // start point
    double mPeriod;
#elif defined(__LINUX__) | defined(__linux__)
    struct timespec mStart;
#endif // defined(WIN32)
};
