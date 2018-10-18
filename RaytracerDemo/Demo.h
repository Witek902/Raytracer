#pragma once

#include "Window.h"

#include "../RaytracerLib/Scene/Scene.h"
#include "../RaytracerLib/Scene/Camera.h"
#include "../RaytracerLib/Mesh/Mesh.h"
#include "../RaytracerLib/Rendering/Viewport.h"
#include "../RaytracerLib/Material/Material.h"
#include "../RaytracerLib/Utils/Bitmap.h"
#include "../RaytracerLib/Rendering/Context.h"
#include "../RaytracerLib/Rendering/PathDebugging.h"
#include "../RaytracerLib/Rendering/DebugRenderer.h"

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
    using Materials = std::vector<std::unique_ptr<rt::Material>>;
    using Meshes = std::vector<std::unique_ptr<rt::Mesh>>;
    using SceneInitCallback = std::function<void(rt::Scene& scene, Materials& materials, Meshes& meshes, CameraSetup& camera)>;

    DemoWindow();
    ~DemoWindow();

    bool Initialize();

    /**
     * Main loop.
     */
    bool Loop();

    void ResetFrame();

private:
    std::unique_ptr<rt::Viewport> mViewport;

    KeyCode mLastKeyDown;

    rt::Camera mCamera;
    rt::RenderingParams mRenderingParams;
    rt::RenderingParams mPreviewRenderingParams;
    rt::PostprocessParams mPostprocessParams;
    CameraSetup mCameraSetup;

    std::map<std::string, SceneInitCallback> mRegisteredScenes;

    Materials mMaterials;
    Meshes mMeshes;
    std::unique_ptr<rt::Scene> mScene;

    Uint32 mFrameNumber;
    Uint32 mFrameCounterForAverage;

    Double mDeltaTime;
    Double mRefreshTime;

    Double mAverageRenderDeltaTime;
    Double mAccumulatedRenderTime;
    Double mRenderDeltaTime;
    Double mPostProcessDeltaTime;
    Double mMinRenderDeltaTime;
    Double mTotalRenderTime;

    Float mCameraSpeed;

    std::unique_ptr<rt::IRenderer> mRenderer;
    std::unique_ptr<rt::DebugRenderer> mDebugRenderer;
    bool mUseDebugRenderer = false;


    // debugging
    rt::PathDebugData mPathDebugData;
    rt::Material* mSelectedMaterial;
    rt::ISceneObject* mSelectedObject;
    Bool mFocalDistancePicking = false;

    void InitializeUI();

    void SwitchScene(const SceneInitCallback& initFunction);
    void RegisterTestScenes();

    void RenderUI();
    void RenderUI_Stats();

    void RenderUI_Debugging();
    void RenderUI_Debugging_Path();
    void RenderUI_Debugging_Color();

    void RenderUI_Settings();
    bool RenderUI_Settings_Rendering();
    bool RenderUI_Settings_Camera();
    bool RenderUI_Settings_PostProcess();
    bool RenderUI_Settings_Object();
    bool RenderUI_Settings_Material();


    virtual void OnMouseDown(MouseButton button, int x, int y) override;
    virtual void OnMouseMove(int x, int y, int deltaX, int deltaY) override;
    virtual void OnMouseUp(MouseButton button) override;
    virtual void OnScroll(int delta) override;
    virtual void OnResize(Uint32 width, Uint32 height) override;
    virtual void OnKeyPress(KeyCode key) override;
    virtual void OnCharTyped(const char* charUTF8) override;

    void ResetCounters();
    void UpdateCamera();
};

extern Options gOptions;