#pragma once

#include "Window.h"
#include "../RaytracerLib/Bitmap.h"
#include "../RaytracerLib/Scene.h"
#include "../RaytracerLib/Camera.h"


struct CameraSetup
{
    rt::math::Vector position;
    Float yaw;
    Float pitch;

    CameraSetup()
        : yaw(0.0f)
        , pitch(0.0f)
    { }
};


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

    CameraSetup mCameraSetup;

    Uint32 mFrameNumber;
    Uint32 mFrameCounterForAverage;

    Double mDeltaTime;
    Double mMinDeltaTime;
    Double mTotalTime;
    Double mRefreshTime;

    void OnMouseDown(Uint32 key, int x, int y) override;
    void OnMouseMove(int x, int y, int deltaX, int deltaY) override;
    void OnMouseUp(Uint32 button) override;

    void OnKeyPress(Uint32 key) override;
    void OnResize(Uint32 width, Uint32 height) override;

    void ResetCounters();
    void ResetCamera();
    bool InitScene();
    void UpdateCamera();
};