#include "PCH.h"
#include "ThreadPool.h"


namespace rt {

ThreadPool::ThreadPool()
    : mNumTasks(0)
    , mCurrentTask(0)
    , mTasksLeft(0)
    , mFinishThreads(true)
{
    StartWorkerThreads(std::thread::hardware_concurrency());
}

ThreadPool::~ThreadPool()
{
    StopWorkerThreads();
}

void ThreadPool::StartWorkerThreads(Uint32 num)
{
    const Uint32 maxThreads = 256;

    if (num > maxThreads)
    {
        num = maxThreads;
    }

    RT_ASSERT(mFinishThreads == true);
    mFinishThreads = false;

    for (Uint32 i = 0; i < num; ++i)
    {
        mThreads.EmplaceBack(&ThreadPool::ThreadCallback, this, i);
    }
}

void ThreadPool::StopWorkerThreads()
{
    RT_ASSERT(mFinishThreads == false);
    mFinishThreads = true;

    {
        Lock lock(mMutex);
        mNewTaskCV.notify_all();
    }

    for (auto& thread : mThreads)
    {
        thread.join();
    }

    mThreads.Clear();
}

void ThreadPool::ThreadCallback(Uint32 threadID)
{
    for (;;)
    {
        Uint32 taskID;

        {
            Lock lock(mMutex);
            while (!mFinishThreads && (mNumTasks == 0))
            {
                mNewTaskCV.wait(lock);
            }

            if (mFinishThreads)
            {
                break;
            }

            taskID = mCurrentTask++;

            // last task
            if (mCurrentTask >= mNumTasks)
            {
                mNumTasks = 0;
            }
        }

        mTask(taskID, threadID);

        if (--mTasksLeft == 0)
        {
            mTileFinishedCV.notify_all();
        }
    }
}

void ThreadPool::SetNumThreads(const Uint32 numThreads)
{
    if (numThreads != GetNumThreads())
    {
        StopWorkerThreads();
        StartWorkerThreads(numThreads);
    }
}

void ThreadPool::RunParallelTask(const ParallelTask& task, Uint32 num)
{
    if (num > 0u)
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
