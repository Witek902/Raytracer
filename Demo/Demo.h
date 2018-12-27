#pragma once

#include "Window.h"

#include "../Core/Scene/Scene.h"
#include "../Core/Scene/Camera.h"
#include "../Core/Mesh/Mesh.h"
#include "../Core/Rendering/Viewport.h"
#include "../Core/Material/Material.h"
#include "../Core/Utils/Bitmap.h"
#include "../Core/Rendering/Context.h"
#include "../Core/Rendering/PathDebugging.h"
#include "../Core/Rendering/DebugRenderer.h"

struct Options
{
    Uint32 windowWidth = 1280;
    Uint32 windowHeight = 720;
    std::string dataPath;

    bool enablePacketTracing = false;
    bool useDebugRenderer = false;

    // TODO JSON scene description
    std::string sceneName;
    std::string modelPath;
    std::string envMapPath;
};

struct RT_ALIGN(16) CameraSetup
{
    rt::math::Vector4 position = rt::math::Vector4::Zero();
    rt::math::Vector4 linearVelocity = rt::math::Vector4::Zero();
    rt::math::Float3 orientation; // yaw, pitch, roll
    rt::math::Float3 angularVelocity;
    Float fov = 60.0f;
};


class RT_ALIGN(64) DemoWindow : public Window
{
public:
    using Materials = std::vector<rt::MaterialPtr>;
    using Meshes = std::vector<rt::MeshPtr>;
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
    rt::Bitmap mImage;

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
    Double mMinimumRenderDeltaTime;
    Double mAccumulatedRenderTime;
    Double mRenderDeltaTime;
    Double mTotalRenderTime;

    Float mCameraSpeed;

    std::unique_ptr<rt::IRenderer> mRenderer;
    std::unique_ptr<rt::DebugRenderer> mDebugRenderer;
    bool mUseDebugRenderer = false;

    bool mEnableUI = true;
    bool mVisualizeAdaptiveRenderingBlocks = false;

    // debugging
    rt::PathDebugData mPathDebugData;
    rt::Material* mSelectedMaterial;
    rt::ISceneObject* mSelectedObject;
    bool mFocalDistancePicking = false;

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
    bool RenderUI_Settings_AdaptiveRendering();
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

    bool IsPreview() const;
    void ResetCounters();
    void UpdateCamera();
};

extern Options gOptions;