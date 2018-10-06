#pragma once

#include "../RayLib.h"

#include "Context.h"
#include "Counters.h"

#include "../Math/Random.h"
#include "../Utils/Bitmap.h"
#include "../Utils/ThreadPool.h"
#include "../Utils/AlignmentAllocator.h"


namespace rt {

class Scene;
class Camera;

struct PostprocessParams
{
    float exposure;
    float ditheringStrength; // applied after tonemapping

    // TODO color correction parameters

    PostprocessParams()
        : exposure(0.0f)
        , ditheringStrength(0.005f)
    { }
};

// Allows for rendering to a window
// TODO make it more generic (e.g. rendering to a texture, etc.)
class RT_ALIGN(64) RAYLIB_API Viewport : public Aligned<64>
{
public:
    Viewport();

    bool Resize(Uint32 width, Uint32 height);
    bool Render(const Scene* scene, const Camera& camera, const RenderingParams& params);
    bool PostProcess(const PostprocessParams& params);
    void Reset();

    RT_FORCE_INLINE const rt::Bitmap& GetFrontBuffer() const { return mFrontBuffer; }

    RT_FORCE_INLINE Uint32 GetNumSamplesRendered() const { return mNumSamplesRendered; }
    RT_FORCE_INLINE Uint32 GetWidth() const { return mSum.GetWidth(); }
    RT_FORCE_INLINE Uint32 GetHeight() const { return mSum.GetHeight(); }

    RT_FORCE_INLINE const RayTracingCounters& GetCounters() const { return mCounters; }

private:
    void InitThreadData();

    // raytrace single image tile (will be called from multiple threads)
    void RenderTile(const Scene& scene, const Camera& camera, RenderingContext& context, Uint32 x0, Uint32 y0);

    // generate "front buffer" image from "average" image
    void PostProcessTile(const PostprocessParams& params, Uint32 ymin, Uint32 ymax, Uint32 threadID);

    ThreadPool mThreadPool;

    std::vector<RenderingContext, AlignmentAllocator<RenderingContext, 64>> mThreadData;

    Bitmap mSum;            // image with accumulated samples (floating point, high dynamic range)
    Bitmap mFrontBuffer;    // postprocesses image (low dynamic range)

    RayTracingCounters mCounters;
    Uint32 mNumSamplesRendered; // number of samples averaged
};

} // namespace rt
