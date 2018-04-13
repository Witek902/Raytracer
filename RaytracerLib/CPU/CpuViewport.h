#pragma once

#include "Viewport.h"
#include "Bitmap.h"
#include "Math/Random.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <functional>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>


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

class CpuScene;
struct RayTracingContext;

struct ThreadData
{
    math::Random random;
    char padding[64];
};

class CpuViewport : public IViewport
{
public:
    CpuViewport();

    // IViewport
    virtual bool Initialize(HWND windowHandle) override;
    virtual bool Resize(Uint32 width, Uint32 height) override;
    virtual bool Render(const IScene* scene, const Camera& camera) override;
    virtual bool SetPostprocessParams(const PostprocessParams& params) override;
    virtual void GetPostprocessParams(PostprocessParams& params) override;
    virtual void Reset() override;

    RT_FORCE_INLINE Uint32 GetWidth() const { return mRenderTarget.GetWidth(); }
    RT_FORCE_INLINE Uint32 GetHeight() const { return mRenderTarget.GetHeight(); }

private:
    void InitThreadData();

    // raytrace single image tile (will be called from multiple threads)
    void RenderTile(const CpuScene& scene, const Camera& camera, RayTracingContext& context,
                    Uint32 x0, Uint32 y0, Uint32 width, Uint32 height);

    // generate "front buffer" image from "average" image
    void PostProcess();

    // paint image into the window
    void Paint();

    ThreadPool mThreadPool;

    HWND mWindow;
    HDC mDC;

    std::vector<ThreadData> mThreadData;

    rt::Bitmap mRenderTarget;   // target image for rendering (floating point)
    rt::Bitmap mSum;            // image with summed up samples (floating point)
    rt::Bitmap mBlurred;        // blurred image (for bloom)
    rt::Bitmap mFrontBuffer;    // image presented on a screen (uchar, post-processed)

    PostprocessParams mPostprocessingParams;
    Uint32 mNumSamplesRendered; // number of samples averaged

    Uint32 mFrameID;
};

} // namespace rt
