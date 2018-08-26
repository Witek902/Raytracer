#pragma once

#include "Window.h"

#include "../RaytracerLib/Scene/Scene.h"
#include "../RaytracerLib/Scene/Camera.h"
#include "../RaytracerLib/Mesh/Mesh.h"
#include "../RaytracerLib/Rendering/Viewport.h"
#include "../RaytracerLib/Material/Material.h"
#include "../RaytracerLib/Utils/Bitmap.h"
#include "../RaytracerLib/Rendering/Context.h"

class Renderer;

struct Options
{
    Uint32 windowWidth = 1280;
    Uint32 windowHeight = 720;
    std::string dataPath;

    // TODO JSON scene description
    std::string modelPath;
    std::string envMapPath;
};

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

    bool Initialize(const Options& options);

    /**
     * Main loop.
     */
    bool Loop();

    /**
     * Reset counters and camera.
     */
    void Reset();

    void ResetFrame();

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

    std::unique_ptr<Renderer> mRenderer;

    // device context
    HDC mDC;

    rt::Material* mSelectedMaterial;

    void InitializeUI();

    void RenderUI();
    void RenderUI_Stats();
    void RenderUI_Settings();
    bool RenderUI_Settings_Rendering();
    bool RenderUI_Settings_Camera();
    bool RenderUI_Settings_PostProcess();
    bool RenderUI_Settings_Material();

    virtual void OnMouseDown(Uint32 key, int x, int y) override;
    virtual void OnMouseMove(int x, int y, int deltaX, int deltaY) override;
    virtual void OnMouseUp(Uint32 button) override;
    virtual void OnScroll(int delta) override;
    virtual void OnResize(Uint32 width, Uint32 height) override;

    void OnKeyPress(Uint32 key) override;

    void ResetCounters();
    void ResetCamera();
    void UpdateCamera();
};