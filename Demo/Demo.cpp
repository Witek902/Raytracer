#include "PCH.h"
#include "Demo.h"
#include "MeshLoader.h"

#include "../Core/Utils/Timer.h"
#include "../Core/Utils/Logger.h"
#include "../Core/Rendering/PathTracer.h"
#include "../Core/Traversal/TraversalContext.h"
#include "../Core/Scene/Object/SceneObject_Mesh.h"
#include "../Core/Scene/Light/BackgroundLight.h"

#include "../External/imgui/imgui_sw.hpp"

using namespace rt;
using namespace math;

DemoWindow::DemoWindow()
    : mLastKeyDown(KeyCode::Invalid)
    , mFrameNumber(0)
    , mDeltaTime(0.0)
    , mRefreshTime(0.0)
    , mAverageRenderDeltaTime(0.0)
    , mMinimumRenderDeltaTime(0.0)
    , mAccumulatedRenderTime(0.0)
    , mRenderDeltaTime(0.0)
    , mTotalRenderTime(0.0)
    , mCameraSpeed(1.0f)
    , mSelectedMaterial(nullptr)
    , mSelectedObject(nullptr)
{
    ResetFrame();
    ResetCounters();
}

DemoWindow::~DemoWindow()
{
    imgui_sw::unbind_imgui_painting();
    ImGui::DestroyContext();
}

bool DemoWindow::Initialize()
{
    RT_LOG_INFO("Using data path: %hs", gOptions.dataPath.c_str());

    if (!Init())
    {
        RT_LOG_ERROR("Failed to init window");
        return false;
    }

    SetSize(gOptions.windowWidth, gOptions.windowHeight);
    SetTitle("Raytracer Demo [Initializing...]");

    if (!Open())
    {
        RT_LOG_ERROR("Failed to open window");
        return false;
    }

    InitializeUI();
    RegisterTestScenes();

    mUseDebugRenderer = gOptions.useDebugRenderer;
    mRenderingParams.numThreads = std::thread::hardware_concurrency();
    mRenderingParams.traversalMode = gOptions.enablePacketTracing ? TraversalMode::Packet : TraversalMode::Single;

    mViewport = std::make_unique<Viewport>();
    mViewport->Resize(gOptions.windowWidth, gOptions.windowHeight);
    mImage.Init(gOptions.windowWidth, gOptions.windowHeight, Bitmap::Format::B8G8R8A8_Uint);

    mCamera.mDOF.aperture = 0.0f;

    const std::string initialSceneName = gOptions.sceneName.empty() ? "Plane" : gOptions.sceneName;
    SwitchScene(mRegisteredScenes[initialSceneName]);

    return true;
}

void DemoWindow::InitializeUI()
{
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImFontConfig fontConfig;
    fontConfig.OversampleH = 4;
    fontConfig.OversampleV = 4;
    fontConfig.PixelSnapH = true;
    ImGui::GetIO().Fonts->AddFontFromFileTTF("../Data/Fonts/DroidSans.ttf", 13, &fontConfig);

    // Setup back-end capabilities flags
    ImGuiIO& io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;   // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;    // We can honor io.WantSetMousePos requests (optional, rarely used)

    // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array that we will update during the application lifetime.
    io.KeyMap[ImGuiKey_Tab] = (int)KeyCode::Tab;
    io.KeyMap[ImGuiKey_LeftArrow] = (int)KeyCode::Left;
    io.KeyMap[ImGuiKey_RightArrow] = (int)KeyCode::Right;
    io.KeyMap[ImGuiKey_UpArrow] = (int)KeyCode::Up;
    io.KeyMap[ImGuiKey_DownArrow] = (int)KeyCode::Down;
    io.KeyMap[ImGuiKey_PageUp] = (int)KeyCode::PageUp;
    io.KeyMap[ImGuiKey_PageDown] = (int)KeyCode::PageDown;
    io.KeyMap[ImGuiKey_Home] = (int)KeyCode::Home;
    io.KeyMap[ImGuiKey_End] = (int)KeyCode::End;
    io.KeyMap[ImGuiKey_Insert] = (int)KeyCode::Insert;
    io.KeyMap[ImGuiKey_Delete] = (int)KeyCode::Delete;
    io.KeyMap[ImGuiKey_Backspace] = (int)KeyCode::Backspace;
    io.KeyMap[ImGuiKey_Space] = (int)KeyCode::Space;
    io.KeyMap[ImGuiKey_Enter] = (int)KeyCode::Enter;
    io.KeyMap[ImGuiKey_Escape] = (int)KeyCode::Escape;
    io.KeyMap[ImGuiKey_A] = (int)KeyCode::A;
    io.KeyMap[ImGuiKey_C] = (int)KeyCode::C;
    io.KeyMap[ImGuiKey_V] = (int)KeyCode::V;
    io.KeyMap[ImGuiKey_X] = (int)KeyCode::X;
    io.KeyMap[ImGuiKey_Y] = (int)KeyCode::Y;
    io.KeyMap[ImGuiKey_Z] = (int)KeyCode::Z;

    ImGui::GetStyle().FramePadding = ImVec2(4, 2);
    ImGui::GetStyle().ItemSpacing = ImVec2(4, 2);
    ImGui::GetStyle().ItemInnerSpacing = ImVec2(2, 2);
    ImGui::GetStyle().ScrollbarSize = 13.0f;
    ImGui::GetStyle().ScrollbarRounding = 0.0f;
    ImGui::GetStyle().WindowRounding = 5.0f;
    ImGui::GetStyle().WindowPadding = ImVec2(6, 6);

    imgui_sw::bind_imgui_painting();
}

void DemoWindow::SwitchScene(const SceneInitCallback& initFunction)
{
    mScene = std::make_unique<Scene>();
    mMaterials.clear();
    mMeshes.clear();

    initFunction(*mScene, mMaterials, mMeshes, mCameraSetup);

    mScene->BuildBVH();
    ResetCounters();
    ResetFrame();

    mSelectedMaterial = nullptr;
    mSelectedObject = nullptr;

    mRenderer = std::make_unique<PathTracer>(*mScene);
    mDebugRenderer = std::make_unique<DebugRenderer>(*mScene);
}

void DemoWindow::ResetFrame()
{
    if (mViewport)
    {
        mViewport->Reset();
    }

    ResetCounters();
}

void DemoWindow::ResetCounters()
{
    mFrameNumber = 0;
    mFrameCounterForAverage = 0;
    mAccumulatedRenderTime = 0.0;
    mAverageRenderDeltaTime = 0.0;
    mMinimumRenderDeltaTime = std::numeric_limits<Double>::max();
    mTotalRenderTime = 0.0;
}

void DemoWindow::OnResize(Uint32 width, Uint32 height)
{
    if (mViewport)
    {
        mViewport->Resize(width, height);
        mImage.Init(width, height, Bitmap::Format::B8G8R8A8_Uint);
    }

    UpdateCamera();
    ResetCounters();
}

void DemoWindow::OnMouseDown(MouseButton button, int x, int y)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MouseDown[(int)button] = true;

    if (io.WantCaptureMouse)
        return;

    Uint32 width, height;
    GetSize(width, height);

    auto renderingContext = std::make_unique<rt::RenderingContext>();

    if (mFocalDistancePicking && button == MouseButton::Left)
    {
        rt::RenderingParams params = mRenderingParams;
        params.antiAliasingSpread = 0.0f;

        renderingContext->params = &params;

        const Vector4 coords((float)x / (float)width, 1.0f - (float)y / (float)height, 0.0f, 0.0f);
        const Ray ray = mCamera.GenerateRay(coords, *renderingContext);

        mRenderer->TraceRay_Single(ray, *renderingContext);
        RT_ASSERT(!mPathDebugData.data.empty());

        HitPoint hitPoint;
        mScene->Traverse_Single({ ray, hitPoint, *renderingContext });

        if (hitPoint.distance == FLT_MAX)
        {
            mCamera.mDOF.focalPlaneDistance = 10000000.0f;
        }
        else
        {
            mCamera.mDOF.focalPlaneDistance = Vector4::Dot3(mCamera.mTransform.GetRotation().GetAxisZ(), ray.dir) * hitPoint.distance;
        }
        ResetFrame();

        mFocalDistancePicking = false;
    }
    else if (button == MouseButton::Left)
    {
        mPathDebugData.data.clear();

        rt::RenderingParams params = mRenderingParams;
        params.antiAliasingSpread = 0.0f;

        renderingContext->params = &params;
        renderingContext->pathDebugData = &mPathDebugData;

        const Vector4 coords((float)x / (float)width, 1.0f - (float)y / (float)height, 0.0f, 0.0f);
        const Ray ray = mCamera.GenerateRay(coords, *renderingContext);

        // TODO DebugPathTracer?
        mRenderer->TraceRay_Single(ray, *renderingContext);
        RT_ASSERT(!mPathDebugData.data.empty());

        HitPoint hitPoint;
        mScene->Traverse_Single({ ray, hitPoint, *renderingContext });

        if (mPathDebugData.data[0].hitPoint.objectId != UINT32_MAX)
        {
            mSelectedMaterial = const_cast<Material*>(mPathDebugData.data[0].shadingData.material);
            mSelectedObject = const_cast<ISceneObject*>(mScene->GetObjects()[mPathDebugData.data[0].hitPoint.objectId].get());
        }
    }
}

void DemoWindow::OnMouseMove(int x, int y, int deltaX, int deltaY)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos.x = (float)x;
    io.MousePos.y = (float)y;

    if (IsMouseButtonDown(MouseButton::Right))
    {
        const Float sensitivity = 0.0001f * mCameraSetup.fov;
        mCameraSetup.orientation.x += sensitivity * (Float)deltaX;
        mCameraSetup.orientation.y -= sensitivity * (Float)deltaY;

        // clamp yaw
        if (mCameraSetup.orientation.x > RT_PI)   mCameraSetup.orientation.x -= 2.0f * RT_PI;
        if (mCameraSetup.orientation.x < -RT_PI)  mCameraSetup.orientation.x += 2.0f * RT_PI;

        // clamp pitch
        if (mCameraSetup.orientation.y > RT_PI * 0.49f)     mCameraSetup.orientation.y = RT_PI * 0.49f;
        if (mCameraSetup.orientation.y < -RT_PI * 0.49f)    mCameraSetup.orientation.y = -RT_PI * 0.49f;
    }
}

void DemoWindow::OnMouseUp(MouseButton button)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MouseDown[(int)button] = false;
}

void DemoWindow::OnScroll(int delta)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheel = (float)delta;

    const float speedMultiplier = 1.25f;

    if (delta > 0)
    {
        mCameraSpeed *= speedMultiplier;
    }
    else if (delta < 0)
    {
        mCameraSpeed /= speedMultiplier;
    }
}

void DemoWindow::OnKeyPress(KeyCode key)
{
    if (key == KeyCode::U)
    {
        mEnableUI = !mEnableUI;
    }

    mLastKeyDown = key;
}

void DemoWindow::OnCharTyped(const char* charUTF8)
{
    ImGuiIO& io = ImGui::GetIO();
    io.AddInputCharactersUTF8(charUTF8);
}

bool DemoWindow::Loop()
{
    Timer localTimer;
    Timer displayTimer;
    displayTimer.Start();

    char buffer[64];

    while (!IsClosed())
    {
        const rt::RenderingProgress& progress = mViewport->GetProgress();
        sprintf(buffer, "Raytracer Demo [%.1f%% converged, pass %u, dt: %.2f]", 100.0f * progress.converged, progress.passesFinished, 1000.0f * mDeltaTime);
        SetTitle(buffer);

        mDeltaTime = displayTimer.Reset();

        UpdateCamera();

        if (IsPreview() && mViewport)
        {
            mPreviewRenderingParams = mRenderingParams;
            mPreviewRenderingParams.antiAliasingSpread = 0.0f;
            mPreviewRenderingParams.samplesPerPixel = 1;
            ResetFrame();
        }

        //// render
        const IRenderer& renderer = mUseDebugRenderer ? (*mDebugRenderer) : (*mRenderer);
        mViewport->SetRenderingParams(IsPreview() ? mPreviewRenderingParams : mRenderingParams);
        localTimer.Start();
        mViewport->Render(renderer, mCamera);
        mRenderDeltaTime = localTimer.Stop();

        if (mEnableUI)
        {
            RenderUI();
        }

        rt::Bitmap::Copy(mImage, mViewport->GetFrontBuffer());

        if (mVisualizeAdaptiveRenderingBlocks)
        {
            mViewport->VisualizeActiveBlocks(mImage);
        }

        // render UI into the front buffer
        if (mEnableUI)
        {
            imgui_sw::paint_imgui((uint32_t*)mImage.GetData(), mImage.GetWidth(), mImage.GetHeight());
        }

        // display pixels in the window
        DrawPixels(mImage.GetData());

        mLastKeyDown = KeyCode::Invalid;

        mTotalRenderTime += mRenderDeltaTime;
        mRefreshTime += mRenderDeltaTime;
        mAccumulatedRenderTime += mRenderDeltaTime;
        mFrameCounterForAverage++;
        mFrameNumber++;
        mAverageRenderDeltaTime = mTotalRenderTime / (double)mFrameCounterForAverage;
        mMinimumRenderDeltaTime = math::Min(mMinimumRenderDeltaTime, mRenderDeltaTime);

        // handle window input
        ProcessMessages();
    }

    return true;
}

bool DemoWindow::IsPreview() const
{
    return IsMouseButtonDown(MouseButton::Right);
}

void DemoWindow::UpdateCamera()
{
    Uint32 width, height;
    GetSize(width, height);

    const Camera oldCameraSetup = mCamera;

    // calculate camera direction from Euler angles
    const Quaternion cameraOrientation = Quaternion::FromAngles(-mCameraSetup.orientation.y, mCameraSetup.orientation.x, mCameraSetup.orientation.z);
    const Vector4 direction = cameraOrientation.GetAxisZ();

    Vector4 movement = math::Vector4::Zero();
    if (IsKeyPressed(KeyCode::W))
        movement += direction;
    if (IsKeyPressed(KeyCode::S))
        movement -= direction;
    if (IsKeyPressed(KeyCode::A))
        movement += Vector4(-direction.z, 0.0f, direction.x, 0.0f);
    if (IsKeyPressed(KeyCode::D))
        movement -= Vector4(-direction.z, 0.0f, direction.x, 0.0f);

    mCamera.mLinearVelocity = mCameraSetup.linearVelocity;

    if (movement.Length3() > RT_EPSILON)
    {
        ResetFrame();

        movement.Normalize3();
        movement *= mCameraSpeed;

        if (IsKeyPressed(KeyCode::ShiftLeft))
            movement *= 5.0f;
        else if (IsKeyPressed(KeyCode::ControlLeft))
            movement /= 5.0f;

        const Vector4 delta = movement * (float)mDeltaTime;
        mCameraSetup.position += delta;
        mCamera.mLinearVelocity -= delta;
    }

    const Float aspectRatio = (Float)width / (Float)height;
    const Float FoV = RT_PI / 180.0f * mCameraSetup.fov;
    mCamera.SetPerspective(Transform(mCameraSetup.position, cameraOrientation), aspectRatio, FoV);

    // rotation motion blur
    if (IsPreview())
    {
        const Quaternion q1 = cameraOrientation.Conjugate();
        const Quaternion q2 = oldCameraSetup.mTransform.GetRotation();

        mCamera.SetAngularVelocity(q1 * q2);
    }
    else
    {
        mCamera.SetAngularVelocity(Quaternion::FromAngles(-mCameraSetup.angularVelocity.y, mCameraSetup.angularVelocity.x, mCameraSetup.angularVelocity.z));
    }
}