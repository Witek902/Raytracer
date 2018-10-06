#include "PCH.h"
#include "Demo.h"
#include "MeshLoader.h"

#include "../RaytracerLib/Mesh/Mesh.h"
#include "../RaytracerLib/Material/Material.h"
#include "../RaytracerLib/Utils/Timer.h"
#include "../RaytracerLib/Utils/Logger.h"
#include "../RaytracerLib/Rendering/Context.h"
#include "../RaytracerLib/Rendering/ShadingData.h"
#include "../RaytracerLib/Traversal/TraversalContext.h"
#include "../RaytracerLib/Scene/SceneObject_Mesh.h"
#include "../RaytracerLib/Scene/SceneObject_Sphere.h"
#include "../RaytracerLib/Scene/SceneObject_Box.h"

#include "../External/imgui/imgui.h"
#include "../External/imgui/imgui_sw.hpp"

using namespace rt;
using namespace math;

DemoWindow::DemoWindow()
    : mLastKeyDown(KeyCode::Invalid)
    , mFrameNumber(0)
    , mDeltaTime(0.0)
    , mRefreshTime(0.0)
    , mAverageRenderDeltaTime(0.0)
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

    mViewport = std::make_unique<Viewport>();
    mViewport->Resize(gOptions.windowWidth, gOptions.windowHeight);

    mCamera.mDOF.aperture = 0.0f;

    SwitchScene(mRegisteredScenes["Simple + Background Light"]);

    /*
    auto loadedMesh = helpers::LoadMesh(gOptions.dataPath + gOptions.modelPath, mMaterials, 1.0f);

    SceneEnvironment env;
    env.backgroundColor = Vector4(2.0f, 2.0f, 2.0f, 0.0f);
    if (!gOptions.envMapPath.empty())
    {
        env.texture = helpers::LoadTexture(gOptions.dataPath, gOptions.envMapPath);
    }
    mScene->SetEnvironment(env);

    if (!gOptions.modelPath.empty())
    {
        SceneObjectPtr meshInstance = std::make_unique<MeshSceneObject>(loadedMesh.get());
        mScene->AddObject(std::move(meshInstance));
    }
    mMeshes.push_back(std::move(loadedMesh));
    */

    return true;
}

void DemoWindow::InitializeUI()
{
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui::GetIO().Fonts->AddFontDefault();

    imgui_sw::bind_imgui_painting();

    //ImGui::GetIO().Fonts->AddFontFromFileTTF("../Data/DroidSans.ttf", 10);
    //ImGui::GetIO().Fonts->GetTexDataAsRGBA32()

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

    //io.ImeWindowHandle = reinterpret_cast<HWND>(GetHandle());
}

void DemoWindow::SwitchScene(const SceneInitCallback& initFunction)
{
    mScene = std::make_unique<Scene>();
    mMaterials.clear();
    mMeshes.clear();
    // TODO clear textures

    initFunction(*mScene, mMaterials, mMeshes, mCameraSetup);

    mScene->BuildBVH();
    ResetCounters();
    ResetFrame();

    mSelectedMaterial = nullptr;
    mSelectedObject = nullptr;
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
    mMinRenderDeltaTime = std::numeric_limits<Double>::max();
    mAccumulatedRenderTime = 0.0;
    mAverageRenderDeltaTime = 0.0;
    mTotalRenderTime = 0.0;
    mPostProcessDeltaTime = 0.0;
}

void DemoWindow::OnResize(Uint32 width, Uint32 height)
{
    if (mViewport)
    {
        mViewport->Resize(width, height);
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

    if (button == MouseButton::Left)
    {
        mPathDebugData.data.clear();

        Uint32 width, height;
        GetSize(width, height);

        rt::RenderingParams params = mRenderingParams;
        params.antiAliasingSpread = 0.0f;

        rt::RenderingContext context;
        context.params = &params;
        context.pathDebugData = &mPathDebugData;

        const Vector4 coords((float)x / (float)width, 1.0f - (float)y / (float)height, 0.0f, 0.0f);
        const Ray ray = mCamera.GenerateRay(coords, context);

        mScene->TraceRay_Single(ray, context);
        assert(!mPathDebugData.data.empty());

        HitPoint hitPoint;
        mScene->Traverse_Single({ ray, hitPoint, context });

        if (mPathDebugData.data[0].hitPoint.objectId != UINT32_MAX)
        {
            mSelectedMaterial = const_cast<Material*>(mPathDebugData.data[0].shadingData.material);
            mSelectedObject = const_cast<ISceneObject*>(mScene->GetObject(mPathDebugData.data[0].hitPoint.objectId));
        }
    }

    /*
    if (button == 0)
    {
        //if (hitPoint.distance == FLT_MAX)
        //{
        //    mCamera.mDOF.focalPlaneDistance = 10000000.0f;
        //}
        //else
        //{
        //    mCamera.mDOF.focalPlaneDistance = Vector4::Dot3(mCamera.mForward, ray.dir) * hitPoint.distance;
        //}
        //ResetFrame();
    }
    */
}

void DemoWindow::OnMouseMove(int x, int y, int deltaX, int deltaY)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos.x = (float)x;
    io.MousePos.y = (float)y;

    if (IsMouseButtonDown(MouseButton::Right))
    {
        const Float sensitivity = 0.0001f * mCameraSetup.fov;
        mCameraSetup.yaw += sensitivity * (Float)deltaX;
        mCameraSetup.pitch -= sensitivity * (Float)deltaY;

        // clamp yaw
        if (mCameraSetup.yaw > RT_PI)   mCameraSetup.yaw -= 2.0f * RT_PI;
        if (mCameraSetup.yaw < -RT_PI)  mCameraSetup.yaw += 2.0f * RT_PI;

        // clamp pitch
        if (mCameraSetup.pitch > RT_PI * 0.49f)     mCameraSetup.pitch = RT_PI * 0.49f;
        if (mCameraSetup.pitch < -RT_PI * 0.49f)    mCameraSetup.pitch = -RT_PI * 0.49f;
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
    if (key == KeyCode::F1)
    {
        SetFullscreenMode(!GetFullscreenMode());
    }

    if (mViewport)
    {
        if (key == KeyCode::R)
        {
            mViewport->Reset();
        }
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
        sprintf(buffer, "Raytracer Demo [Sample %u]", mViewport->GetNumSamplesRendered());
        SetTitle(buffer);

        mDeltaTime = displayTimer.Reset();

        ProcessMessages();
        UpdateCamera();

        const bool isPreview = IsMouseButtonDown(MouseButton::Right);
        if (isPreview && mViewport)
        {
            mPreviewRenderingParams = mRenderingParams;
            mPreviewRenderingParams.antiAliasingSpread = 0.0f;
            mPreviewRenderingParams.samplesPerPixel = 1;

            ResetFrame();
        }

        // render
        localTimer.Start();
        mViewport->Render(mScene.get(), mCamera, isPreview ? mPreviewRenderingParams : mRenderingParams);
        mRenderDeltaTime = localTimer.Stop();

        //// post process
        localTimer.Start();
        mViewport->PostProcess(mPostprocessParams);
        mPostProcessDeltaTime = localTimer.Stop();

        RenderUI();

        const rt::Bitmap& frontBuffer = mViewport->GetFrontBuffer();

        imgui_sw::paint_imgui((uint32_t*)frontBuffer.GetData(), frontBuffer.GetWidth(), frontBuffer.GetHeight());

        DrawPixels(frontBuffer.GetData());

        mLastKeyDown = KeyCode::Invalid;

        mTotalRenderTime += mRenderDeltaTime;
        mRefreshTime += mRenderDeltaTime;
        mAccumulatedRenderTime += mRenderDeltaTime;
        mMinRenderDeltaTime = Min(mMinRenderDeltaTime, mRenderDeltaTime);
        mFrameCounterForAverage++;
        mFrameNumber++;
        mAverageRenderDeltaTime = mTotalRenderTime / (double)mFrameCounterForAverage;
    }

    return true;
}

void DemoWindow::UpdateCamera()
{
    Uint32 width, height;
    GetSize(width, height);

    const Camera oldCameraSetup = mCamera;

    // calculate camera direction from Euler angles
    const Quaternion cameraOrientation = Quaternion::FromAngles(-mCameraSetup.pitch, mCameraSetup.yaw, 0.0f);
    const Vector4 direction = cameraOrientation.GetAxisZ();

    Vector4 movement;
    if (IsKeyPressed(KeyCode::W))
        movement += direction;
    if (IsKeyPressed(KeyCode::S))
        movement -= direction;
    if (IsKeyPressed(KeyCode::A))
        movement += Vector4(-direction.z, 0.0f, direction.x, 0.0f);
    if (IsKeyPressed(KeyCode::D))
        movement -= Vector4(-direction.z, 0.0f, direction.x, 0.0f);

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
        mCamera.mLinearVelocity = -delta;
    }
    else
    {
        mCamera.mLinearVelocity = Vector4();
    }

    const Float aspectRatio = (Float)width / (Float)height;
    const Float FoV = RT_PI / 180.0f * mCameraSetup.fov;
    mCamera.SetPerspective(Transform(mCameraSetup.position, cameraOrientation), aspectRatio, FoV);

    // rotation motion blur
    if (mFrameNumber > 0)
    {
        const Quaternion q1 = cameraOrientation.Conjugate();
        const Quaternion q2 = oldCameraSetup.mTransform.GetRotation();

        mCamera.mAngularVelocity = (q1 * q2).Normalized();
    }
    else
    {
        mCamera.mAngularVelocity = Quaternion::Identity();
    }
}