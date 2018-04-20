#pragma once

#include "../Common.h"

#include <functional>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <vector>

namespace rt {

using ParallelTask = std::function<void(Uint32 x, Uint32 y, Uint32 threadID)>;

class ThreadPool
{
public:
    struct TaskCoords
    {
        Uint32 x;
        Uint32 y;
    };

    ThreadPool();
    ~ThreadPool();

    void RunParallelTask(const ParallelTask& task, Uint32 rows, Uint32 columns);

private:
    using Lock = std::unique_lock<std::mutex>;

    std::vector<std::thread> mThreads;
    std::condition_variable mNewTaskCV;
    std::condition_variable mTileFinishedCV;
    std::mutex mMutex;

    ParallelTask mTask;
    Uint32 mCurrentX, mCurrentY;
    Uint32 mRows, mColumns;

    std::atomic<Uint32> mTilesLeftToComplete;

    bool mFinishThreads;

    void ThreadCallback(Uint32 id);
};

} // namespace rt
