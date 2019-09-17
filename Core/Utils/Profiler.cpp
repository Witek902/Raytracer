#include "PCH.h"
#include "Profiler.h"
#include "Timer.h"
#include "Logger.h"

#include <string>
#include <map>

namespace rt {

Profiler& Profiler::GetInstance()
{
    static Profiler profiler;
    return profiler;
}

void Profiler::RegisterEntry(ScopedEntry& entry)
{
    std::lock_guard<std::mutex> lock(mLock);
    entry.nextEntry = mFirstEntry;
    mFirstEntry = &entry;
}

void Profiler::Collect(DynArray<ProfilerResult>& outResult)
{
    std::map<const char*, ScopedEntryData> entriesMap;
    {
        std::lock_guard<std::mutex> lock(mLock);
        for (ScopedEntry* entry = mFirstEntry; entry != nullptr; entry = entry->nextEntry)
        {
            entriesMap[entry->name] += entry->data;
        }
    }

    for (const auto& iter : entriesMap)
    {
        ProfilerResult result;
        result.scopeName = iter.first;
        result.avgTime = iter.second.accumulatedTicks * gTimerPeriod / iter.second.count;
        result.minTime = iter.second.minTick * gTimerPeriod;
        result.count = iter.second.count;
        outResult.PushBack(result);
        //RT_LOG_INFO("Profiler: %s count=%llu, avg=%.3f us, min=%.3fus, ", iter.first, data.count, avgTime, minTime);
    }
}

void Profiler::ResetAll()
{
    std::lock_guard<std::mutex> lock(mLock);
    for (ScopedEntry* entry = mFirstEntry; entry != nullptr; entry = entry->nextEntry)
    {
        entry->data = ScopedEntryData();
    }
}

ScopedEntry::ScopedEntry(const char* name)
    : name(name)
{
    Profiler::GetInstance().RegisterEntry(*this);
}


} // namespace rt
