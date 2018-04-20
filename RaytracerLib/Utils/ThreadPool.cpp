#include "PCH.h"
#include "ThreadPool.h"


namespace rt {

ThreadPool::ThreadPool()
    : mFinishThreads(false)
    , mRows(0)
    , mColumns(0)
    , mCurrentX(0)
    , mCurrentY(0)
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

void ThreadPool::ThreadCallback(Uint32 id)
{
    for (;;)
    {
        Uint32 x, y;

        {
            Lock lock(mMutex);
            while (!mFinishThreads && (mRows == 0 || mColumns == 0))
                mNewTaskCV.wait(lock);

            if (mFinishThreads)
                break;

            x = mCurrentX++;
            y = mCurrentY;
            if (mCurrentX >= mColumns)
            {
                mCurrentX = 0;
                mCurrentY++;
            }

            // last tile
            if (mCurrentY >= mRows)
            {
                mRows = 0;
                mColumns = 0;
            }
        }

        mTask(x, y, id);

        {
            if (--mTilesLeftToComplete == 0)
                mTileFinishedCV.notify_all();
        }
    }
}

void ThreadPool::RunParallelTask(const ParallelTask& task, Uint32 rows, Uint32 columns)
{
    {
        Lock lock(mMutex);

        mTask = task;
        mCurrentX = 0;
        mCurrentY = 0;
        mRows = rows;
        mColumns = columns;
        mTilesLeftToComplete = rows * columns;

        mNewTaskCV.notify_all();

        mTileFinishedCV.wait(lock);
    }
}

} // namespace rt
