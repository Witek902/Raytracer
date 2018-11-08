#pragma once

#include "../Common.h"

#include <functional>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <vector>

namespace rt {

using ParallelTask = std::function<void(Uint32 taskID, Uint32 threadID)>;

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

    void RunParallelTask(const ParallelTask& task, Uint32 num);

    RT_FORCE_INLINE Uint32 GetNumThreads() const
    {
        return static_cast<Uint32>(mThreads.size());
    }

private:
    using Lock = std::unique_lock<std::mutex>;

    std::vector<std::thread> mThreads;
    std::condition_variable mNewTaskCV;
    std::condition_variable mTileFinishedCV;
    std::mutex mMutex;

    ParallelTask mTask;
    Uint32 mNumTasks;
    Uint32 mCurrentTask;

    std::atomic<Uint32> mTasksLeft;

    bool mFinishThreads;

    void ThreadCallback(Uint32 id);
};

} // namespace rt
