#pragma once

#include "RayLib.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>


namespace rt {

class IScene;
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

/**
 * Interface representing rendering viewport.
 * This interface is also responsible for merging Monte Carlo samples
 * and generating final, post-processed image.
 */
class RAYLIB_API IViewport
{
public:
    virtual ~IViewport() { }

    virtual bool Initialize(HWND windowHandle) = 0;

    virtual bool Resize(Uint32 width, Uint32 height) = 0;

    // Perform scene raytracing
    // Result will be stored in a internal temporary buffer
    // Averaged and post-processed image will be displayed in a window
    virtual bool Render(const IScene* scene, const Camera& camera) = 0;

    virtual bool SetPostprocessParams(const PostprocessParams& params) = 0;
    virtual void GetPostprocessParams(PostprocessParams& params) = 0;

    // Reset averaged samples
    virtual void Reset() = 0;
};


} // namespace rt
