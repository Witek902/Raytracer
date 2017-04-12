#pragma once

#include "Window.h"
#include "../RaytracerLib/Bitmap.h"
#include "../RaytracerLib/Scene.h"
#include "../RaytracerLib/Camera.h"


class DemoWindow : public Window
{
public:
    DemoWindow();

    bool Initialize();

    /**
     * Main loop.
     */
    bool Loop();

    /**
     * Reset counters.
     */
    void Reset();

    void Render();

private:
    rt::Bitmap mFramebuffer;
    rt::Camera mCamera;
    std::unique_ptr<rt::Scene> mScene;

    Uint32 mFrameNumber;
    Uint32 mFrameCounterForAverage;

    Double mDeltaTime;
    Double mTotalTime;
    Double mRefreshTime;

    void OnKeyPress(Uint32 key) override;
    void OnResize(Uint32 width, Uint32 height) override;

    bool InitScene();
    void UpdateCamera();
};