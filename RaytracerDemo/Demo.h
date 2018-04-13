#pragma once

#include "Window.h"
#include "../RaytracerLib/Bitmap.h"
#include "../RaytracerLib/Scene.h"
#include "../RaytracerLib/Camera.h"
#include "../RaytracerLib/Mesh.h"
#include "../RaytracerLib/Instance.h"
#include "../RaytracerLib/Viewport.h"
#include "../RaytracerLib/Material.h"


struct RT_ALIGN(16) CameraSetup
{
    rt::math::Vector4 position;
    Float yaw;
    Float pitch;

    CameraSetup()
        : yaw(0.0f)
        , pitch(0.0f)
    { }
};


class RT_ALIGN(16) DemoWindow : public Window
{
public:
    DemoWindow(rt::Instance& instance);

    bool Initialize();

    bool InitScene();

    /**
     * Main loop.
     */
    bool Loop();

    /**
     * Reset counters and camera.
     */
    void Reset();

    void Render();

private:
    std::unique_ptr<rt::IViewport> mViewport;
    rt::Camera mCamera;

    rt::Instance& mInstance;

    // TODO move to Main
    std::vector<std::unique_ptr<rt::Material>> mMaterials;
    std::unique_ptr<rt::IMesh> mMesh;
    std::unique_ptr<rt::IScene> mScene;

    Float mCameraSpeed;
    CameraSetup mCameraSetup;

    Uint32 mFrameNumber;
    Uint32 mFrameCounterForAverage;

    Double mDeltaTime;
    Double mMinDeltaTime;
    Double mTotalTime;
    Double mRefreshTime;

    virtual void OnMouseDown(Uint32 key, int x, int y) override;
    virtual void OnMouseMove(int x, int y, int deltaX, int deltaY) override;
    virtual void OnMouseUp(Uint32 button) override;
    virtual void OnScroll(int delta) override;

    void OnKeyPress(Uint32 key) override;
    void OnResize(Uint32 width, Uint32 height) override;

    void ResetCounters();
    void ResetCamera();
    void UpdateCamera();
};