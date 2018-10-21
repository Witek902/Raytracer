#pragma once

#include "../RayLib.h"

#include "Context.h"
#include "Counters.h"

#include "../Math/Random.h"
#include "../Utils/Bitmap.h"
#include "../Utils/ThreadPool.h"
#include "../Utils/AlignmentAllocator.h"


namespace rt {

class IRenderer;
class Camera;

struct PostprocessParams
{
    math::Vector4 colorFilter = math::VECTOR_ONE;

    // exposure in log scale
    float exposure = 0.0f;

    // applied after tonemapping
    float ditheringStrength = 0.005f;
};


class RT_ALIGN(64) RAYLIB_API Viewport : public Aligned<64>
{
public:
    Viewport();

    bool Resize(Uint32 width, Uint32 height);
    bool Render(const IRenderer& renderer, const Camera& camera, const RenderingParams& params);
    bool PostProcess(const PostprocessParams& params);
    void Reset();

    RT_FORCE_INLINE const rt::Bitmap& GetFrontBuffer() const { return mFrontBuffer; }
    RT_FORCE_INLINE const rt::Bitmap& GetAccumulatedBuffer() const { return mAccumulated; }

    RT_FORCE_INLINE Uint32 GetNumSamplesRendered() const { return mNumSamplesRendered; }
    RT_FORCE_INLINE Uint32 GetWidth() const { return mAccumulated.GetWidth(); }
    RT_FORCE_INLINE Uint32 GetHeight() const { return mAccumulated.GetHeight(); }

    RT_FORCE_INLINE const RayTracingCounters& GetCounters() const { return mCounters; }

private:
    void InitThreadData();

    struct TileRenderingContext
    {
        const IRenderer& renderer;
        const Camera& camera;
        const math::Vector4 sampleOffset;
    };

    // raytrace single image tile (will be called from multiple threads)
    void RenderTile(const TileRenderingContext& tileContext, RenderingContext& renderingContext, Uint32 x0, Uint32 y0);

    // generate "front buffer" image from "average" image
    void PostProcessTile(const PostprocessParams& params, Uint32 ymin, Uint32 ymax, Uint32 threadID);

    ThreadPool mThreadPool;

    std::vector<RenderingContext, AlignmentAllocator<RenderingContext, 64>> mThreadData;

    Bitmap mAccumulated;    // image with accumulated samples (floating point, high dynamic range)
    Bitmap mFrontBuffer;    // postprocesses image (low dynamic range)

    RayTracingCounters mCounters;
    Uint32 mNumSamplesRendered; // number of samples averaged
};

} // namespace rt
