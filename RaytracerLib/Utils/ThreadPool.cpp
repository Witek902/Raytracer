#include "PCH.h"
#include "ThreadPool.h"


namespace rt {

ThreadPool::ThreadPool()
    : mNumTasks(0)
    , mCurrentTask(0)
    , mTasksLeft(0)
    , mFinishThreads(false)
{
    const Uint32 numThreads = std::thread::hardware_concurrency();

    // start worker threads
    for (size_t i = 0; i < numThreads; ++i)
    {
        mThreads.emplace_back(std::thread(&ThreadPool::ThreadCallback, this, static_cast<Uint32>(i)));
    }
}

ThreadPool::~ThreadPool()
{
    {
        Lock lock(mMutex);
        mFinishThreads = true;
        mNewTaskCV.notify_all();
    }

    for (auto& thread : mThreads)
    {
        thread.join();
    }
}

void ThreadPool::ThreadCallback(Uint32 threadID)
{
    for (;;)
    {
        Uint32 taskID;

        {
            Lock lock(mMutex);
            while (!mFinishThreads && (mNumTasks == 0))
                mNewTaskCV.wait(lock);

            if (mFinishThreads)
                break;

            taskID = mCurrentTask++;

            // last task
            if (mCurrentTask >= mNumTasks)
            {
                mNumTasks = 0;
            }
        }

        mTask(taskID, threadID);

        {
            if (--mTasksLeft == 0)
                mTileFinishedCV.notify_all();
        }
    }
}

void ThreadPool::RunParallelTask(const ParallelTask& task, Uint32 num)
{
    {
        Lock lock(mMutex);

        mTask = task;
        mCurrentTask = 0;
        mNumTasks = num;
        mTasksLeft = num;

        mNewTaskCV.notify_all();

        mTileFinishedCV.wait(lock);
    }
}

} // namespace rt
