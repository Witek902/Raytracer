#pragma once

#include "../RayLib.h"
#include "../Containers/DynArray.h"
#include "../Math/Math.h"

#include <mutex>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

namespace rt {

class ScopedEntry;

struct ProfilerResult
{
    const char* scopeName = nullptr;
    double avgTime = 0.0;
    double minTime = 0.0;
    Uint64 count = 0;
};

class RAYLIB_API Profiler
{
public:
    static Profiler& GetInstance();
    void RegisterEntry(ScopedEntry& entry);
    void Collect(DynArray<ProfilerResult>& outResult);
    void ResetAll();

private:
    Profiler() = default;

    std::mutex mLock;
    ScopedEntry* mFirstEntry;
};

struct ScopedEntryData
{
    Uint64 minTick = UINT64_MAX;
    Uint64 accumulatedTicks = 0;
    Uint64 count = 0;

    ScopedEntryData& operator += (const ScopedEntryData& other)
    {
        minTick = math::Min(minTick, other.accumulatedTicks);
        accumulatedTicks += other.accumulatedTicks;
        count += other.count;
        return *this;
    }
};

class RAYLIB_API ScopedEntry
{
public:
    RT_FORCE_NOINLINE ScopedEntry(const char* name);

    const char* name;
    ScopedEntry* nextEntry = nullptr;
    ScopedEntryData data;
};


class RAYLIB_API ScopedTimer
{
public:
    RT_FORCE_INLINE ScopedTimer(ScopedEntry& entry)
        : mEntry(entry)
    {
        QueryPerformanceCounter(&mStart);
    }

    RT_FORCE_INLINE ~ScopedTimer()
    {
        LARGE_INTEGER stop;
        QueryPerformanceCounter(&stop);
        const Uint64 ticks = stop.QuadPart - mStart.QuadPart;

        if (ticks > 0)
        {
            mEntry.data.minTick = math::Min(mEntry.data.minTick, ticks);
        }

        mEntry.data.accumulatedTicks += ticks;
        mEntry.data.count++;
    }

    void ReportAndReset();

private:
    LARGE_INTEGER mStart;
    ScopedEntry& mEntry;
};

} // namespace rt


#define RT_SCOPED_TIMER(name) \
    thread_local ScopedEntry entry##name(#name); \
    ScopedTimer scopedTimer(entry##name);
