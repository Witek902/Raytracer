#pragma once

#include "../RayLib.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>


/**
 * High resolution timer.
 */
class RAYLIB_API Timer
{
private:
    LARGE_INTEGER mStart;
    double mPeriod;

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
};
