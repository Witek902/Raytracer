#pragma once

#include "Window.h"

#include "../RaytracerLib/Scene/Scene.h"
#include "../RaytracerLib/Scene/Camera.h"
#include "../RaytracerLib/Mesh/Mesh.h"
#include "../RaytracerLib/Rendering/Viewport.h"
#include "../RaytracerLib/Material/Material.h"
#include "../RaytracerLib/Utils/Bitmap.h"
#include "../RaytracerLib/Rendering/Context.h"


struct RT_ALIGN(16) CameraSetup
{
    rt::math::Vector4 position;
    Float yaw;
    Float pitch;
    Float fov;

    CameraSetup()
        : yaw(0.0f)
        , pitch(0.0f)
        , fov(60.0f)
    { }
};


class RT_ALIGN(16) DemoWindow : public Window
{
public:
    DemoWindow();
    ~DemoWindow();

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

    void ResetFrame();

    void Render();

private:
    std::unique_ptr<rt::Viewport> mViewport;

    rt::Camera mCamera;
    rt::RenderingParams mRenderingParams;
    rt::PostprocessParams mPostprocessParams;
    CameraSetup mCameraSetup;

    // TODO move to Main
    std::vector<std::unique_ptr<rt::Material>> mMaterials;
    std::vector<std::unique_ptr<rt::Mesh>> mMeshes;
    std::unique_ptr<rt::Scene> mScene;

    Float mCameraSpeed;

    Uint32 mFrameNumber;
    Uint32 mFrameCounterForAverage;

    Double mDeltaTime;
    Double mRefreshTime;

    Double mAverageRenderDeltaTime;
    Double mAccumulatedRenderTime;
    Double mRenderDeltaTime;
    Double mMinRenderDeltaTime;
    Double mTotalRenderTime;

    // device context
    HDC mDC;

    rt::Material* mSelectedMaterial;

    void InitializeUI();
    void RenderUI();

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