#include "PCH.h"
#include "Demo.h"
#include "MeshLoader.h"
#include "Renderer.h"

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

#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>

using namespace rt;
using namespace math;

DemoWindow::DemoWindow()
    : mFrameNumber(0)
    , mDeltaTime(0.0)
    , mRenderDeltaTime(0.0)
    , mTotalRenderTime(0.0)
    , mRefreshTime(0.0)
    , mAccumulatedRenderTime(0.0)
    , mAverageRenderDeltaTime(0.0)
    , mCameraSpeed(1.0f)
    , mSelectedMaterial(nullptr)
    , mSelectedObject(nullptr)
    , mLastKeyDown(0)
{
    Reset();
}

DemoWindow::~DemoWindow()
{
    ImGui_ImplDX11_Shutdown();
    ImGui::DestroyContext();
}

bool DemoWindow::Initialize(const Options& options)
{
    if (!Init())
    {
        RT_LOG_ERROR("Failed to init window");
        return false;
    }

    SetSize(options.windowWidth, options.windowHeight);
    SetTitle("Raytracer Demo [Initializing...]");

    if (!Open())
    {
        RT_LOG_ERROR("Failed to open window");
        return false;
    }

    InitializeUI();

    mViewport = std::make_unique<Viewport>();
    mViewport->Resize(options.windowWidth, options.windowHeight);

    mDC = GetDC(reinterpret_cast<HWND>(GetHandle()));

    mRenderer = std::make_unique<Renderer>();
    if (!mRenderer->Init(reinterpret_cast<HWND>(GetHandle())))
    {
        RT_LOG_ERROR("Failed to initialize renderer");
        return false;
    }

    mCamera.mDOF.aperture = 0.0f;

    Random random;

   // initialize scene
    {
        RT_LOG_INFO("Using data path: %hs", options.dataPath.c_str());

        mScene = std::make_unique<Scene>();

        auto loadedMesh = helpers::LoadMesh(options.dataPath + options.modelPath, mMaterials, 1.0f);

        SceneEnvironment env;
        env.backgroundColor = Vector4(2.0f, 2.0f, 2.0f, 0.0f);
        if (!options.envMapPath.empty())
        {
            env.texture = helpers::LoadTexture(options.dataPath, options.envMapPath);
        }
        mScene->SetEnvironment(env);

        {
            auto mesh = helpers::CreatePlaneMesh(mMaterials, 500.0f, 1.0f);
            SceneObjectPtr instance = std::make_unique<MeshSceneObject>(mesh.get());
            instance->mTransform.SetTranslation(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
            mScene->AddObject(std::move(instance));
            mMeshes.push_back(std::move(mesh));
        }

        {
            auto material = std::make_unique<rt::Material>();
            material->debugName = "light";
            material->emissionColor = math::Vector4(20.0f, 20.0f, 20.0f, 0.0f);
            material->light = true;
            material->Compile();

            SceneObjectPtr instance = std::make_unique<SphereSceneObject>(0.5f, material.get());
            instance->mTransform.SetTranslation(Vector4(0.0f, 0.5f, -2.0f, 0.0f));
            mScene->AddObject(std::move(instance));
            mMaterials.push_back(std::move(material));
        }

        {
            auto material = std::make_unique<rt::Material>();
            material->debugName = "default";
            material->baseColor = math::Vector4(0.9f, 0.9f, 0.9f, 0.0f);
            material->roughness = 0.2f;
            material->transparent = false;
            material->Compile();

            SceneObjectPtr instance = std::make_unique<SphereSceneObject>(0.5f, material.get());
            instance->mTransform.SetTranslation(Vector4(-1.5f, 0.5f, 0.0f, 0.0f));
            mScene->AddObject(std::move(instance));
            mMaterials.push_back(std::move(material));
        }

        {
            auto material = std::make_unique<rt::Material>();
            material->debugName = "glass";
            material->baseColor = math::Vector4(1.0f, 1.0f, 1.0f, 0.0f);
            material->roughness = 0.001f;
            material->transparent = true;
            material->Compile();

            SceneObjectPtr instance = std::make_unique<SphereSceneObject>(0.5f, material.get());
            instance->mTransform.SetTranslation(Vector4(0.0f, 0.5f, 0.0f, 0.0f));
            mScene->AddObject(std::move(instance));
            mMaterials.push_back(std::move(material));
        }

        {
            auto material = std::make_unique<rt::Material>();
            material->debugName = "mirror";
            material->baseColor = math::Vector4(1.0f, 1.0f, 1.0f, 0.0f);
            material->roughness = 0.002f;
            material->metalness = 1.0f;
            material->Compile();

            SceneObjectPtr instance = std::make_unique<SphereSceneObject>(0.5f, material.get());
            instance->mTransform.SetTranslation(Vector4(1.5f, 0.5f, 0.0f, 0.0f));
            mScene->AddObject(std::move(instance));
            mMaterials.push_back(std::move(material));
        }

        /*
        if (!options.modelPath.empty())
        {
            SceneObjectPtr meshInstance = std::make_unique<MeshSceneObject>(loadedMesh.get());
            mScene->AddObject(std::move(meshInstance));
        }
        */

        mMeshes.push_back(std::move(loadedMesh));
        mScene->BuildBVH();
    }

    return true;
}

void DemoWindow::InitializeUI()
{
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui::GetIO().Fonts->AddFontDefault();

    //ImGui::GetIO().Fonts->AddFontFromFileTTF("../Data/DroidSans.ttf", 10);
    //ImGui::GetIO().Fonts->GetTexDataAsRGBA32()

    // Setup back-end capabilities flags
    ImGuiIO& io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;   // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;    // We can honor io.WantSetMousePos requests (optional, rarely used)

    // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array that we will update during the application lifetime.
    io.KeyMap[ImGuiKey_Tab] = VK_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
    io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
    io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
    io.KeyMap[ImGuiKey_Home] = VK_HOME;
    io.KeyMap[ImGuiKey_End] = VK_END;
    io.KeyMap[ImGuiKey_Insert] = VK_INSERT;
    io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
    io.KeyMap[ImGuiKey_Space] = VK_SPACE;
    io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
    io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
    io.KeyMap[ImGuiKey_A] = 'A';
    io.KeyMap[ImGuiKey_C] = 'C';
    io.KeyMap[ImGuiKey_V] = 'V';
    io.KeyMap[ImGuiKey_X] = 'X';
    io.KeyMap[ImGuiKey_Y] = 'Y';
    io.KeyMap[ImGuiKey_Z] = 'Z';

    io.ImeWindowHandle = reinterpret_cast<HWND>(GetHandle());
}

void DemoWindow::Reset()
{
    ResetCounters();
    ResetCamera();
}

void DemoWindow::ResetFrame()
{
    mViewport->Reset();

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
}

void DemoWindow::OnResize(Uint32 width, Uint32 height)
{
    if (mRenderer)
    {
        mRenderer->OnWindowResized(width, height);
    }

    if (mViewport)
    {
        mViewport->Resize(width, height);
    }

    mDC = GetDC(reinterpret_cast<HWND>(GetHandle()));

    UpdateCamera();
    ResetCounters();
}

void DemoWindow::ResetCamera()
{
    // cornell box
    mCameraSetup.position = Vector4(0.02f, 0.3f, 3.2f, 0.0f);
    mCameraSetup.pitch = -0.1f;
    mCameraSetup.yaw = -3.11f;

    ////mCameraSetup.position = Vector4(0.0f, 0.5f, 0.0f, 0.0f);
    //mCameraSetup.pitch = -1.0f;
    //mCameraSetup.yaw = 0.01f;

    // sponza
    //mCameraSetup.position = Vector4(700.0f, 300.0f, 10.0f, 0.0f);
    //mCameraSetup.pitch = -0.09f;
    //mCameraSetup.yaw = -1.73f;

    //mCameraSetup.position = Vector4(0.0f, 0.75f, 0.0f, 0.0f);
    //mCameraSetup.pitch = -1.70f;
    //mCameraSetup.yaw = -1.73f;
}

void DemoWindow::OnMouseDown(Uint32 button, int x, int y)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MouseDown[button] = true;

    if (io.WantCaptureMouse)
        return;

    if (button == 0)
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

        const rt::Color color = mScene->TraceRay_Single(ray, context);
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

    if (IsMouseButtonDown(1))
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

void DemoWindow::OnMouseUp(Uint32 button)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MouseDown[button] = false;
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

void DemoWindow::OnKeyPress(Uint32 key)
{
    if (key == VK_F1)
    {
        SetFullscreenMode(!GetFullscreenMode());
    }

    if (mViewport)
    {
        if (key == 'R')
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
    Timer renderTimer;
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

        const bool isPreview = IsMouseButtonDown(1);
        if (isPreview && mViewport)
        {
            mPreviewRenderingParams = mRenderingParams;
            mPreviewRenderingParams.antiAliasingSpread = 0.0f;
            mPreviewRenderingParams.samplesPerPixel = 1;

            ResetFrame();
        }

        // render
        renderTimer.Start();
        mViewport->Render(mScene.get(), mCamera, isPreview ? mPreviewRenderingParams : mRenderingParams);
        mRenderDeltaTime = renderTimer.Stop();

        RenderUI();
        mLastKeyDown = 0;

        mRenderer->Render((const float*)mViewport->GetAccumulatedBuffer().GetData(), mPostprocessParams, mViewport->GetNumSamplesRendered());

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
    const float cosPitch = cosf(mCameraSetup.pitch);
    const Vector4 direction = Vector4(sinf(mCameraSetup.yaw) * cosPitch,
                                      sinf(mCameraSetup.pitch),
                                      cosf(mCameraSetup.yaw) * cosPitch,
                                      0.0f);

    Vector4 movement;
    if (IsKeyPressed('W'))
        movement += direction;
    if (IsKeyPressed('S'))
        movement -= direction;
    if (IsKeyPressed('A'))
        movement += Vector4(-direction[2], 0.0f, direction[0], 0.0f);
    if (IsKeyPressed('D'))
        movement -= Vector4(-direction[2], 0.0f, direction[0], 0.0f);

    if (movement.Length3() > RT_EPSILON)
    {
        ResetFrame();

        movement.Normalize3();
        movement *= mCameraSpeed;

        if (IsKeyPressed(VK_LSHIFT))
            movement *= 5.0f;
        else if (IsKeyPressed(VK_LCONTROL))
            movement /= 5.0f;

        mCameraSetup.position += movement * (Float)mDeltaTime;
    }

    mCamera.SetPerspective(mCameraSetup.position, direction,
                           Vector4(0.0f, 1.0f, 0.0f, 0.0f),
                           (Float)width / (Float)height,
                           RT_PI / 180.0f * mCameraSetup.fov);

    mCamera.Update();
}