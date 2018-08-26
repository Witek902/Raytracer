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
#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>

#define DISABLE_IMGUI

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
    InitializeUI();

    if (!Init())
    {
        RT_LOG_ERROR("Failed to init window");
        return false;
    }

    SetSize(options.windowWidth, options.windowHeight);
    SetTitle("Raytracer Demo");

    if (!Open())
    {
        RT_LOG_ERROR("Failed to open window");
        return false;
    }

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
    mCamera.barrelDistortionFactor = 0.0f;

    Random random;

   // initialize scene
    {
        RT_LOG_INFO("Using data path: %hs", options.dataPath.c_str());

        mScene = std::make_unique<Scene>();

        auto loadedMesh = helpers::LoadMesh(options.dataPath + options.modelPath, mMaterials, 1.0f);

        if (!options.envMapPath.empty())
        {
            SceneEnvironment env;
            env.texture = helpers::LoadTexture(options.dataPath, options.envMapPath);
            mScene->SetEnvironment(env);
        }

        /*
        {
            auto mesh = helpers::CreatePlaneMesh(mMaterials, 100.0f, 0.4f);
            SceneObjectPtr meshInstance = std::make_unique<MeshSceneObject>(mesh.get());
            mScene->AddObject(std::move(meshInstance));
            mMeshes.push_back(std::move(mesh));
        }

        {
            auto material = std::make_unique<rt::Material>();
            material->debugName = "glass";
            material->baseColor = math::Vector4(1.0f, 1.0f, 1.0f, 0.0f);
            material->roughness = 0.001f;
            material->transparent = true;
            material->Compile();

            SceneObjectPtr meshInstance = std::make_unique<BoxSceneObject>(math::Vector4(1.0f, 1.0f, 1.0f, 0.0f), material.get());
           // meshInstance->mTransform.SetTranslation(Vector4(0.0f, 2.005f, 0.0f, 0.0f));
            mScene->AddObject(std::move(meshInstance));
            mMaterials.push_back(std::move(material));
        }*/

        if (!options.modelPath.empty())
        {
            SceneObjectPtr meshInstance = std::make_unique<MeshSceneObject>(loadedMesh.get());
            mScene->AddObject(std::move(meshInstance));
        }

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
    mCameraSetup.position = Vector4(0.02f, 1.1f, 1.8f, 0.0f);
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
        Uint32 width, height;
        GetSize(width, height);

        rt::RenderingParams params;
        params.antiAliasingSpread = 0.0f;
        rt::RenderingContext context(&params);

        const Vector4 coords((float)x / (float)width, 1.0f - (float)y / (float)height, 0.0f, 0.0f);
        const Ray ray = mCamera.GenerateRay(coords, context);

        HitPoint hitPoint;
        mScene->Traverse_Single({ ray, hitPoint, context });

        if (hitPoint.objectId != UINT32_MAX)
        {
            ShadingData shadingData;
            mScene->ExtractShadingData(ray.origin, ray.dir, hitPoint, shadingData);

            mSelectedMaterial = const_cast<Material*>(shadingData.material);
        }

        /*
        if (hitPoint.distance == FLT_MAX)
        {
            mCamera.mDOF.focalPlaneDistance = 10000000.0f;
        }
        else
        {
            mCamera.mDOF.focalPlaneDistance = Vector4::Dot3(mCamera.mForward, ray.dir) * hitPoint.distance;
        }

        ResetFrame();
        */
    }
}

void DemoWindow::OnMouseMove(int x, int y, int deltaX, int deltaY)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos.x = (float)x;
    io.MousePos.y = (float)y;

    if (IsMouseButtonDown(1))
    {
        const Float sensitivity = 0.01f;
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
}

bool DemoWindow::Loop()
{
    Timer renderTimer;
    Timer displayTimer;
    displayTimer.Start();

    while (!IsClosed())
    {
        mDeltaTime = displayTimer.Reset();

        ProcessMessages();
        UpdateCamera();

        if (IsMouseButtonDown(1) && mViewport)
        {
            ResetFrame();
        }

        // render
        renderTimer.Start();
        mViewport->Render(mScene.get(), mCamera, mRenderingParams);
        mRenderDeltaTime = renderTimer.Stop();

        RenderUI();

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

void DemoWindow::RenderUI_Stats()
{
    ImGui::Text("Average render time: %.2f ms", 1000.0 * mAverageRenderDeltaTime);
    ImGui::Text("Minimum render time: %.2f ms", 1000.0 * mMinRenderDeltaTime);
    ImGui::Text("Total render time:   %.3f s", mTotalRenderTime);
    ImGui::Text("Samples rendered:    %u", mViewport->GetNumSamplesRendered());
    ImGui::Text("Frame number:        %u", mFrameNumber);

    ImGui::Separator();

    ImGui::Text("Delta time: %.2f ms", 1000.0 * mDeltaTime);

    ImGui::Separator();

    const RayTracingCounters& counters = mViewport->GetCounters();
#ifdef RT_ENABLE_INTERSECTION_COUNTERS
    ImGui::Text("Ray-box tests (total):  %.2fM", (float)counters.numRayBoxTests / 1000000.0f);
    ImGui::Text("Ray-box tests (passed): %.2fM", (float)counters.numPassedRayBoxTests / 1000000.0f);
    ImGui::Text("Ray-tri tests (total):  %.2fM", (float)counters.numRayTriangleTests / 1000000.0f);
    ImGui::Text("Ray-tri tests (passed): %.2fM", (float)counters.numPassedRayTriangleTests / 1000000.0f);
#endif // RT_ENABLE_INTERSECTION_COUNTERS
}

void DemoWindow::RenderUI_Settings()
{
    bool resetFrame = false;

    if (ImGui::TreeNode("Rendering"))
    {
        resetFrame |= RenderUI_Settings_Rendering();
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Camera"))
    {
        resetFrame |= RenderUI_Settings_Camera();
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Postprocess"))
    {
        resetFrame |= RenderUI_Settings_PostProcess();
        ImGui::TreePop();
    }

    if (mSelectedMaterial)
    {
        if (ImGui::TreeNode("Material", "Material (%s)", mSelectedMaterial->debugName.c_str()))
        {
            resetFrame |= RenderUI_Settings_Material();
            ImGui::TreePop();
        }
    }

    if (resetFrame)
    {
        ResetFrame();
    }
}

bool DemoWindow::RenderUI_Settings_Rendering()
{
    bool resetFrame = false;

    int renderingModeIndex = static_cast<int>(mRenderingParams.renderingMode);
    int traversalModeIndex = static_cast<int>(mRenderingParams.traversalMode);
    int tileOrder = static_cast<int>(mRenderingParams.tileOrder);

    const char* renderingModeItems[] =
    {
        "Regular", "BaseColor",
        "Depth", "Position", "Normals", "Tangents", "Bitangents", "TexCoords", "TriangleID",
#ifdef RT_ENABLE_INTERSECTION_COUNTERS
        "RayBoxIntersection", "RayBoxIntersectionPassed", "RayTriIntersection", "RayTriIntersectionPassed",
#endif // RT_ENABLE_INTERSECTION_COUNTERS
    };
    resetFrame |= ImGui::Combo("Rendering mode", &renderingModeIndex, renderingModeItems, IM_ARRAYSIZE(renderingModeItems));

    const char* traversalModeItems[] = { "Single", "SIMD", "Packet" };
    resetFrame |= ImGui::Combo("Traversal mode", &traversalModeIndex, traversalModeItems, IM_ARRAYSIZE(traversalModeItems));

    ImGui::SliderInt("Tile order", (int*)&tileOrder, 0, 8); // max 256x256 tile

    resetFrame |= ImGui::SliderInt("Max ray depth", (int*)&mRenderingParams.maxRayDepth, 1, 50);
    ImGui::SliderInt("Samples per pixel", (int*)&mRenderingParams.samplesPerPixel, 1, 64);
    resetFrame |= ImGui::SliderInt("Russian roulette depth", (int*)&mRenderingParams.minRussianRouletteDepth, 1, 64);
    resetFrame |= ImGui::SliderFloat("Antialiasing spread", &mRenderingParams.antiAliasingSpread, 0.0f, 3.0f);

    mRenderingParams.renderingMode = static_cast<RenderingMode>(renderingModeIndex);
    mRenderingParams.traversalMode = static_cast<TraversalMode>(traversalModeIndex);
    mRenderingParams.tileOrder = static_cast<Uint8>(tileOrder);

    return resetFrame;
}

bool DemoWindow::RenderUI_Settings_Camera()
{
    bool resetFrame = false;

    const char* bokehTypeNames[] = { "Circle", "Hexagon", "Box" };
    int bokehTypeIndex = static_cast<int>(mCamera.mDOF.bokehType);

    resetFrame |= ImGui::InputFloat3("Position", &mCameraSetup.position.x, 3);

    resetFrame |= ImGui::SliderFloat("Field of view", &mCameraSetup.fov, 0.5f, 120.0f);
    resetFrame |= ImGui::SliderFloat("Aperture", &mCamera.mDOF.aperture, 0.0f, 0.1f);
    resetFrame |= ImGui::SliderFloat("Focal distance", &mCamera.mDOF.focalPlaneDistance, 0.1f, 1000.0f, "%.3f", 2.0f);
    resetFrame |= ImGui::Combo("Bokeh Shape", &bokehTypeIndex, bokehTypeNames, IM_ARRAYSIZE(bokehTypeNames));
    resetFrame |= ImGui::SliderFloat("Barrel distortion", &mCamera.barrelDistortionFactor, 0.0f, 0.2f);

    mCamera.mDOF.bokehType = static_cast<BokehShape>(bokehTypeIndex);

    return resetFrame;
}

bool DemoWindow::RenderUI_Settings_PostProcess()
{
    ImGui::SliderFloat("Exposure", &mPostprocessParams.exposure, -8.0f, 8.0f, "%+.3f EV");
    ImGui::SliderFloat("Dithering", &mPostprocessParams.ditheringStrength, 0.0f, 0.1f);

    return false;
}

bool DemoWindow::RenderUI_Settings_Material()
{
    bool materialChanged = false;

    materialChanged |= ImGui::Checkbox("Transparent", &mSelectedMaterial->transparent);
    materialChanged |= ImGui::ColorEdit3("Emission color", &mSelectedMaterial->emissionColor[0], ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
    materialChanged |= ImGui::ColorEdit3("Base color", &mSelectedMaterial->baseColor[0], ImGuiColorEditFlags_Float);
    materialChanged |= ImGui::SliderFloat("Roughness", &mSelectedMaterial->roughness, 0.0f, 1.0f);
    materialChanged |= ImGui::SliderFloat("Metalness", &mSelectedMaterial->metalness, 0.0f, 1.0f);
    materialChanged |= ImGui::SliderFloat("Refractive index", &mSelectedMaterial->IoR, 0.0f, 6.0f);

    if (mSelectedMaterial->metalness > 0.0f)
    {
        materialChanged |= ImGui::SliderFloat("Extinction coefficient", &mSelectedMaterial->K, 0.0f, 10.0f);
    }

    if (materialChanged)
    {
        mSelectedMaterial->Compile();
    }

    return materialChanged;
}

void DemoWindow::RenderUI()
{
    ImGui_ImplDX11_NewFrame();

    Uint32 width, height;
    GetSize(width, height);

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);
    io.DeltaTime = (float)mDeltaTime;
    io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
    io.KeySuper = false;

    ImGui::NewFrame();
    {
        static bool showStats = true;
        if (ImGui::Begin("Stats", &showStats))
        {
            RenderUI_Stats();
        }
        ImGui::End();

        static bool showRenderSettings = false;
        if (ImGui::Begin("Settings", &showRenderSettings))
        {
            RenderUI_Settings();
        }
        ImGui::End();
    }
    ImGui::EndFrame();

    ImGui::Render();
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