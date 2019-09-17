#include "PCH.h"
#include "Timer.h"

namespace rt {

#ifdef WIN32

static double GetCounterPeriod()
{
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    return 1.0 / static_cast<double>(freq.QuadPart);
}

const double gTimerPeriod = GetCounterPeriod();

#endif // WIN32

Timer::Timer()
{
#if defined(WIN32)
    mPeriod = gTimerPeriod;
#elif defined(__LINUX__) | defined(__linux__)
    mStart.tv_sec = 0;
    mStart.tv_nsec = 0;
#endif // defined(WIN32)

    Start();
}

double Timer::Stop()
{
#if defined(WIN32)

    LARGE_INTEGER stop;
    QueryPerformanceCounter(&stop);
    return static_cast<double>(stop.QuadPart - mStart.QuadPart) * mPeriod;

#elif defined(__LINUX__) | defined(__linux__)

    struct timespec mStop;
    clock_gettime(CLOCK_MONOTONIC, &mStop);

    // negative time difference should not occur when using CLOCK_MONOTONIC
    time_t sec_result = (mStop.tv_sec - mStart.tv_sec) * 1000000000;
    return (double)(sec_result + (mStop.tv_nsec - mStart.tv_nsec)) * 1e-9;

#endif // defined(WIN32)
}

double Timer::Reset()
{
#if defined(WIN32)

    LARGE_INTEGER stop;
    QueryPerformanceCounter(&stop);
    const double t = static_cast<double>(stop.QuadPart - mStart.QuadPart) * mPeriod;
    mStart = stop;
    return t;

#elif defined(__LINUX__) | defined(__linux__)

    struct timespec stop;
    clock_gettime(CLOCK_MONOTONIC, &stop);

    // negative time difference should not occur when using CLOCK_MONOTONIC
    time_t sec_result = (stop.tv_sec - mStart.tv_sec) * 1000000000;
    double result = (double)(sec_result + (stop.tv_nsec - mStart.tv_nsec)) * 1e-9;
    mStart = stop;
    return result;

#endif // defined(WIN32)
}

} // namespace rt
