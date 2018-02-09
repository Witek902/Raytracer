#include "PCH.h"
#include "Timer.h"

namespace {

static double GetCounterPeriod()
{
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    return 1.0 / static_cast<double>(freq.QuadPart);
}

} // namespace

Timer::Timer()
{
    static double gPeriod = GetCounterPeriod();
    mPeriod = gPeriod;

    mStart.QuadPart = 0;
}

void Timer::Start()
{
    QueryPerformanceCounter(&mStart);
}

double Timer::Stop()
{
    LARGE_INTEGER stop;
    QueryPerformanceCounter(&stop);

    return static_cast<double>(stop.QuadPart - mStart.QuadPart) * mPeriod;
}

double Timer::Reset()
{
    LARGE_INTEGER stop;
    QueryPerformanceCounter(&stop);
    const double t = static_cast<double>(stop.QuadPart - mStart.QuadPart) * mPeriod;
    mStart = stop;
    return t;
}
