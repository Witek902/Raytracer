#pragma once

#include "../Common.h"
#include "../Containers/DynArray.h"

#include <functional>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>

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

    void SetNumThreads(const Uint32 numThreads);

    void RunParallelTask(const ParallelTask& task, Uint32 num);

    RT_FORCE_INLINE Uint32 GetNumThreads() const
    {
        return mThreads.Size();
    }

private:

    void StartWorkerThreads(Uint32 num);
    void StopWorkerThreads();

    using Lock = std::unique_lock<std::mutex>;

    DynArray<std::thread> mThreads;
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
