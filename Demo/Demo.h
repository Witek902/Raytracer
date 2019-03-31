#pragma once

#include "Window.h"

#include "../Core/Scene/Scene.h"
#include "../Core/Scene/Camera.h"
#include "../Core/Mesh/Mesh.h"
#include "../Core/Rendering/Viewport.h"
#include "../Core/Material/Material.h"
#include "../Core/Utils/Texture.h"
#include "../Core/Rendering/Context.h"
#include "../Core/Rendering/PathDebugging.h"

struct Options
{
    Uint32 windowWidth = 1280;
    Uint32 windowHeight = 720;
    std::string dataPath;

    Uint32 numThreads = 0;

    bool enablePacketTracing = false;
    std::string rendererName = "Path Tracer";

    std::string sceneName;
};

struct RT_ALIGN(16) CameraSetup
{
    rt::math::Vector4 position = rt::math::Vector4::Zero();
    rt::math::Vector4 linearVelocity = rt::math::Vector4::Zero();
    rt::math::Float3 orientation; // yaw, pitch, roll
    rt::math::Float3 angularVelocity;
    float fov = 60.0f;
};


class RT_ALIGN(64) DemoWindow : public Window
{
public:
    using Materials = std::vector<rt::MaterialPtr>;
    using Meshes = std::vector<rt::MeshPtr>;

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
    float mCameraSpeed;

    Materials mMaterials;
    Meshes mMeshes;
    std::unique_ptr<rt::Scene> mScene;

    Uint32 mFrameNumber;
    Uint32 mFrameCounterForAverage;

    double mDeltaTime;
    double mRefreshTime;

    double mAverageRenderDeltaTime;
    double mMinimumRenderDeltaTime;
    double mAccumulatedRenderTime;
    double mRenderDeltaTime;
    double mTotalRenderTime;

    std::string mSceneFileName;
    time_t mSceneFileModificationTime;

    std::string mRendererName;
    rt::RendererPtr mRenderer;

    bool mEnableUI = true;
    bool mVisualizeAdaptiveRenderingBlocks = false;

    // debugging
    rt::PathDebugData mPathDebugData;
    rt::Material* mSelectedMaterial;
    rt::ISceneObject* mSelectedObject;
    bool mFocalDistancePicking = false;
    bool mPixelDebuggingPicking = false;

    void InitializeUI();

    void CheckSceneFileModificationTime();
    void SwitchScene(const std::string& sceneName);

    bool RenderUI();
    void RenderUI_Stats();

    void RenderUI_Debugging();
    void RenderUI_Debugging_Path();
    void RenderUI_Debugging_Color();

    bool RenderUI_Settings();
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
    virtual void OnFileDrop(const std::string& filePath) override;

    bool IsPreview() const;
    void ResetCounters();
    void UpdateCamera();
};

extern Options gOptions;