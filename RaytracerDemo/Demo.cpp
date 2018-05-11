#include "PCH.h"
#include "Demo.h"
#include "MeshLoader.h"

#include "../RaytracerLib/Math/Matrix.h"
#include "../RaytracerLib/Mesh/Mesh.h"
#include "../RaytracerLib/Material/Material.h"

#include "../RaytracerLib/Utils/Timer.h"
#include "../RaytracerLib/Utils/Logger.h"
#include "../RaytracerLib/Rendering/Context.h"

#include <imgui/imgui.h>
#include <imgui/imgui.h>
#include <imgui/imgui_sw.hpp>


using namespace rt;
using namespace math;

namespace {

Uint32 WINDOW_WIDTH = 1280;
Uint32 WINDOW_HEIGHT = 720;

}

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
    imgui_sw::unbind_imgui_painting();
    ImGui::DestroyContext();
}

bool DemoWindow::Initialize()
{
    InitializeUI();

    if (!Init())
    {
        RT_LOG_ERROR("Failed to init window");
        return false;
    }

    SetSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    SetTitle("Raytracer Demo");

    if (!Open())
    {
        RT_LOG_ERROR("Failed to open window");
        return false;
    }

    mViewport = std::make_unique<Viewport>();
    mViewport->Resize(WINDOW_WIDTH, WINDOW_HEIGHT);

    mDC = GetDC(reinterpret_cast<HWND>(GetHandle()));

    return true;
}

void DemoWindow::InitializeUI()
{
    IMGUI_CHECKVERSION();
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
    mCameraSetup.position = Vector4(0.0f, 0.2f, 1.0f);
    mCameraSetup.pitch = -0.1f;
    mCameraSetup.yaw = -3.13f;


    // sponza
    mCameraSetup.position = Vector4(7.0f, 3.0f, 0.1f);
    mCameraSetup.pitch = -0.09f;
    mCameraSetup.yaw = -1.73f;

    //mCameraSetup.position = Vector4(0.0f, 0.75f, 0.0f);
    //mCameraSetup.pitch = -1.70f;
    //mCameraSetup.yaw = -1.73f;
}

bool DemoWindow::InitScene()
{
    // EVERYTHING HERE IS TEMPORARY !

    //auto bunny = helpers::LoadMesh("../../../../MODELS/bunny.obj", mMaterials);
    //auto cornellBox = helpers::LoadMesh("../../../../MODELS/CornellBox/CornellBox-Original.obj", mMaterials);
    //mMesh = helpers::LoadMesh("../../../../MODELS/CornellBox/CornellBox-Mirror.obj", mMaterials);
    //mMesh = helpers::LoadMesh("../../../../MODELS/living_room/living_room.obj", mMaterials);
    auto cubeMesh = helpers::LoadMesh("../../../../MODELS/cube/cube.obj", mMaterials);
    auto sponza = helpers::LoadMesh("../../../../MODELS/crytek-sponza/sponza.obj", mMaterials, 0.01f);
    auto planeMesh = helpers::CreatePlaneMesh(mMaterials, 100.0f);

    // SCENE
    {
        mScene = std::make_unique<Scene>();

        SceneEnvironment env;
        env.texture = helpers::LoadTexture("../../../../TEXTURES/", "ENV/Topanga_Forest_B_3k.dds");
        env.backgroundColor = Vector4::Splat(1.0f);
        mScene->SetEnvironment(env);

        {
            SceneObjectPtr meshInstance = std::make_unique<MeshSceneObject>(sponza.get());
            meshInstance->mPosition = Vector4(0.0f, 0.0f, 0.0f);
            mScene->AddObject(std::move(meshInstance));
        }

        /*
        {
            auto material = std::make_unique<Material>("plasticB");
            material->baseColor = Vector4(0.95f, 0.212f, 0.95f);
            material->roughness = 0.1f;
            material->emissionColor = Vector4();
            material->Compile();
            SceneObjectPtr meshInstance = std::make_unique<SphereSceneObject>(0.5f, material.get());
            meshInstance->mPosition = Vector4(1.0f, 0.5f, -1.0f);
            mScene->AddObject(std::move(meshInstance));

            mMaterials.push_back(std::move(material));
        }

        {
            Material* material = new Material;
            material->emissionColor = Vector4(6.0f, 6.0f, 6.0f);
            material->Compile();
            SceneObjectPtr meshInstance = std::make_unique<SphereSceneObject>(0.5f, material);
            meshInstance->mPosition = Vector4(0.241f, 0.0f, -3.0f);
            mScene->AddObject(std::move(meshInstance));
        }
        */
    }

    mScene->BuildBVH();

    //mMeshes.push_back(std::move(bunny));
    mMeshes.push_back(std::move(cubeMesh));
    mMeshes.push_back(std::move(planeMesh));
    mMeshes.push_back(std::move(sponza));

    // TODO remove
    mSelectedMaterial = mMaterials.back().get();

    return true;
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
        rt::RenderingContext context(&params);

        const Vector4 coords((float)x / (float)width, 1.0f - (float)y / (float)height);
        const Ray ray = mCamera.GenerateRay(coords, context);

        HitPoint hitPoint;
        mScene->Traverse_Single(ray, hitPoint, context);

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
        else if (key == VK_OEM_4) // [
        {
            PostprocessParams params;
            mViewport->GetPostprocessParams(params);
            params.exposure /= 1.1f;
            mViewport->SetPostprocessParams(params);
        }
        else if (key == VK_OEM_6) // ]
        {
            PostprocessParams params;
            mViewport->GetPostprocessParams(params);
            params.exposure *= 1.1f;
            mViewport->SetPostprocessParams(params);
        }
        else if (key == 'P') // printscreen
        {

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

        mViewport->SetPostprocessParams(mPostprocessParams);

        // render
        renderTimer.Start();
        mViewport->Render(mScene.get(), mCamera, mRenderingParams);
        mRenderDeltaTime = renderTimer.Stop();

        RenderUI();

        Render();

        mTotalRenderTime += mRenderDeltaTime;
        mRefreshTime += mRenderDeltaTime;
        mAccumulatedRenderTime += mRenderDeltaTime;
        mMinRenderDeltaTime = Min(mMinRenderDeltaTime, mRenderDeltaTime);
        mFrameCounterForAverage++;
        mFrameNumber++;
        mAverageRenderDeltaTime = mTotalRenderTime / (double)mFrameCounterForAverage;

        /*
        // refresh averages
        mRefreshTime += mDeltaTime;
        if (mRefreshTime > 1.0)
        {
            mAccumulatedRenderTime = 0.0;
            mFrameCounterForAverage = 0;
            mRefreshTime = 0.0;
        }
        */
    }

    return true;
}

void DemoWindow::RenderUI()
{
    static bool test = false;
    static imgui_sw::SwOptions sw_options;

    Uint32 width, height;
    GetSize(width, height);

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);
    io.DeltaTime = (float)mDeltaTime;
    io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
    io.KeySuper = false;

    static int renderingModeIndex = 3;

    ImGui::NewFrame();
    {
        static bool showStats = true;
        if (ImGui::Begin("Stats", &showStats))
        {
            ImGui::Text("Average render time: %.2f ms", 1000.0 * mAverageRenderDeltaTime);
            ImGui::Text("Minimum render time: %.2f ms", 1000.0 * mMinRenderDeltaTime);
            ImGui::Text("Total render time:   %.3f s", mTotalRenderTime);
            ImGui::Text("Samples rendered:    %u", mViewport->GetNumSamplesRendered());
            ImGui::Text("Frame number:        %u", mFrameNumber);
            ImGui::Separator();
            ImGui::Text("Delta time: %.2f ms", 1000.0 * mDeltaTime);
        }
        ImGui::End();

        static bool showRenderSettings = false;
        if (ImGui::Begin("Settings", &showRenderSettings))
        {
            bool resetFrame = false;

            if (ImGui::TreeNode("Rendering"))
            {
                const char* items[] =
                {
                    "Regular",
                    "Depth", "Position", "Normals", "Tangents", "Bitangents", "TexCoords",
                    "BaseColor",
                    "RayBoxIntersection", "RayBoxIntersectionPassed", "RayTriIntersection", "RayTriIntersectionPassed",
                };
                resetFrame |= ImGui::Combo("Mode", &renderingModeIndex, items, IM_ARRAYSIZE(items));


                resetFrame |= ImGui::SliderInt("Max ray depth", (int*)&mRenderingParams.maxRayDepth, 1, 50);
                ImGui::SliderInt("Samples per pixel", (int*)&mRenderingParams.samplesPerPixel, 1, 64);
                resetFrame |= ImGui::SliderInt("Russian roulette depth", (int*)&mRenderingParams.minRussianRouletteDepth, 1, 64);
                resetFrame |= ImGui::SliderFloat("Antialiasing spread", &mRenderingParams.antiAliasingSpread, 0.0f, 3.0f);

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Camera"))
            {
                resetFrame |= ImGui::SliderFloat("Field of view", &mCameraSetup.fov, 0.5f, 120.0f);
                resetFrame |= ImGui::SliderFloat("Aperture", &mCamera.mDOF.aperture, 0.0f, 0.1f);
                resetFrame |= ImGui::SliderFloat("Focal distance", &mCamera.mDOF.focalPlaneDistance, 0.1f, 1000.0f, "%.3f", 2.0f);
                resetFrame |= ImGui::SliderFloat("Barrel distortion", &mCamera.barrelDistortionFactor, 0.0f, 0.2f);

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Postprocess"))
            {
                ImGui::SliderFloat("Exposure", &mPostprocessParams.exposure, 0.01f, 10.0f, "%.3f", 2.0f);
                ImGui::SliderFloat("Film grain", &mPostprocessParams.filmGrainStrength, 0.0f, 0.1f);
                ImGui::SliderFloat("Dithering", &mPostprocessParams.ditheringStrength, 0.0f, 0.1f);
                ImGui::SliderFloat("Bloom strength", &mPostprocessParams.bloomStrength, 0.0f, 1.0f);
                ImGui::SliderFloat("Bloom size", &mPostprocessParams.bloomSize, 0.0f, 50.0f);

                ImGui::TreePop();
            }

            if (mSelectedMaterial)
            {
                if (ImGui::TreeNode("Material", "Material (%s)", mSelectedMaterial->debugName.c_str()))
                {
                    bool materialChanged = false;

                    materialChanged |= ImGui::Checkbox("Metal", &mSelectedMaterial->metal);
                    materialChanged |= ImGui::ColorEdit3("Emission color", &mSelectedMaterial->emissionColor[0], ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
                    materialChanged |= ImGui::ColorEdit3("Base color", &mSelectedMaterial->baseColor[0], ImGuiColorEditFlags_Float);
                    materialChanged |= ImGui::SliderFloat("Roughness", &mSelectedMaterial->roughness, 0.0f, 1.0f);
                    materialChanged |= ImGui::SliderFloat("Refractive index", &mSelectedMaterial->IoR, 0.0f, 6.0f);
                    if (mSelectedMaterial->metal)
                    {
                        materialChanged |= ImGui::SliderFloat("Extinction coefficient", &mSelectedMaterial->K, 0.0f, 10.0f);
                    }

                    if (materialChanged)
                    {
                        mSelectedMaterial->Compile();
                    }

                    ImGui::TreePop();
                    resetFrame |= materialChanged;
                }
            }

            if (ImGui::Button("Take screenshot"))
            {
                mViewport->GetFrontBuffer().SaveBMP("screenshot.bmp", true);
            }

            if (resetFrame)
            {
                ResetFrame();
            }
        }
        ImGui::End();
    }
    ImGui::EndFrame();

    ImGui::Render();

    const rt::Bitmap& frontBuffer = mViewport->GetFrontBuffer();
    paint_imgui((uint32_t*)frontBuffer.GetData(), frontBuffer.GetWidth(), frontBuffer.GetHeight(), sw_options);

    mRenderingParams.renderingMode = static_cast<rt::RenderingMode>(renderingModeIndex);
}

void DemoWindow::Render()
{
    Uint32 width, height;
    GetSize(width, height);

    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = 4 * width * height;
    bmi.bmiHeader.biXPelsPerMeter = 1;
    bmi.bmiHeader.biYPelsPerMeter = 1;

    const rt::Bitmap& frontBuffer = mViewport->GetFrontBuffer();

    if (0 == StretchDIBits(mDC,
                           0, height, width, (Uint32)(-(Int32)height), // flip image
                           0, 0, width, height,
                           frontBuffer.GetData(),
                           &bmi, DIB_RGB_COLORS, SRCCOPY))
    {
        RT_LOG_ERROR("Paint failed");
    }
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
                                      cosf(mCameraSetup.yaw) * cosPitch);

    Vector4 movement;
    if (IsKeyPressed('W'))
        movement += direction;
    if (IsKeyPressed('S'))
        movement -= direction;
    if (IsKeyPressed('A'))
        movement += Vector4(-direction[2], 0.0f, direction[0]);
    if (IsKeyPressed('D'))
        movement -= Vector4(-direction[2], 0.0f, direction[0]);

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
                           Vector4(0.0f, 1.0f, 0.0f),
                           (Float)width / (Float)height,
                           RT_PI / 180.0f * mCameraSetup.fov);

    mCamera.Update();
}