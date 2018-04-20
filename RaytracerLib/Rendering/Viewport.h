#pragma once

#include "../RayLib.h"

#include "Settings.h"

#include "../Utils/Bitmap.h"
#include "../Utils/ThreadPool.h"
#include "../Math/Random.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>


namespace rt {

class Scene;
class Camera;

struct PostprocessParams
{
    float exposure;
    float bloomStrength;
    float bloomSize;
    float noiseStrength; // dithering

    // TODO color correction parameters

    PostprocessParams()
        : exposure(0.8f)
        , bloomSize(20.0f)
        , bloomStrength(0.0f)
        , noiseStrength(0.025f)
    { }
};

struct ThreadData
{
    math::Random random;
    char padding[64];
};

// Allows for rendering to a window
// TODO make it more generic (e.g. rendering to a texture, etc.)
class RAYLIB_API Viewport
{
public:
    Viewport();

    bool Initialize(HWND windowHandle);
    bool Resize(Uint32 width, Uint32 height);
    bool Render(const Scene* scene, const Camera& camera);
    bool SetPostprocessParams(const PostprocessParams& params);
    void GetPostprocessParams(PostprocessParams& params);
    void Reset();

    RT_FORCE_INLINE Uint32 GetWidth() const { return mRenderTarget.GetWidth(); }
    RT_FORCE_INLINE Uint32 GetHeight() const { return mRenderTarget.GetHeight(); }

private:
    void InitThreadData();

    // raytrace single image tile (will be called from multiple threads)
    void RenderTile(const Scene& scene, const Camera& camera, RenderingContext& context,
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
