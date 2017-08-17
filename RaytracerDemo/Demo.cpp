#include "PCH.h"
#include "Demo.h"
#include "Timer.h"
#include "MeshLoader.h"
#include "../RaytracerLib/Logger.h"
#include "../RaytracerLib/Mesh.h"
#include "../RaytracerLib/Material.h"


using namespace rt;

namespace {

Uint32 WINDOW_WIDTH = 640;
Uint32 WINDOW_HEIGHT = 360;

}

DemoWindow::DemoWindow(rt::Instance& instance)
    : mFrameNumber(0)
    , mDeltaTime(0.0)
    , mTotalTime(0.0)
    , mRefreshTime(0.0)
    , mInstance(instance)
{
    Reset();
}

bool DemoWindow::Initialize()
{
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

    mViewport = mInstance.CreateViewport();
    mViewport->Initialize(reinterpret_cast<HWND>(GetHandle()));
    mViewport->Resize(WINDOW_WIDTH, WINDOW_HEIGHT);
    return true;
}

void DemoWindow::Reset()
{
    ResetCounters();
    ResetCamera();
    InitScene();
}

void DemoWindow::ResetCounters()
{
    mFrameNumber = 0;
    mFrameCounterForAverage = 0;
    mMinDeltaTime = std::numeric_limits<Double>::max();
    mDeltaTime = 0.0;
    mTotalTime = 0.0;
    mRefreshTime = 0.0;
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

void DemoWindow::ResetCamera()
{
    mCameraSetup.position = rt::math::Vector4(0.001f, 0.001f, -2.0f);
    mCameraSetup.pitch = 0.01f;
    mCameraSetup.yaw = 0.01f;
}

bool DemoWindow::InitScene()
{
    mMesh = helpers::LoadMesh("../../../MODELS/CornellBox/CornellBox-Original.obj", mInstance, mMaterials);
    if (!mMesh)
        return false;

    // SCENE
    {
        mScene = mInstance.CreateScene();

        MeshInstance meshInstance;
        meshInstance.mMesh = mMesh.get();
        mScene->CreateMeshInstance(meshInstance);
    }

    return true;
}

void DemoWindow::OnMouseDown(Uint32 key, int x, int y)
{
    RT_UNUSED(x);
    RT_UNUSED(y);
}

void DemoWindow::OnMouseMove(int x, int y, int deltaX, int deltaY)
{
    if (IsMouseButtonDown(0))
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
    }
}

bool DemoWindow::Loop()
{
    Timer timer;
    timer.Start();

    while (!IsClosed())
    {
        ProcessMessages();
        UpdateCamera();

        if (IsMouseButtonDown(0) && mViewport)
        {
            mViewport->Reset();
        }

        Double dt;
        {
            timer.Start();
            mViewport->Render(mScene.get(), mCamera);
            dt = timer.Stop();
        }

        mDeltaTime = dt;
        mTotalTime += dt;
        mRefreshTime += dt;
        mFrameNumber++;
        mFrameCounterForAverage++;
        mMinDeltaTime = math::Min(mMinDeltaTime, mDeltaTime);

        // refresh window title bar
        if (mRefreshTime > 1.0)
        {
            const double avgDtMs = 1000.0 * mRefreshTime / static_cast<Double>(mFrameCounterForAverage);
            const double minDtMs = 1000.0 * mMinDeltaTime;

            std::stringstream stringStream;
            stringStream << "Raytracer Demo " << "dt = " << avgDtMs << "ms (min. " << minDtMs << "), " << "frame " << mFrameNumber;
            SetTitle(stringStream.str().c_str());

            mFrameCounterForAverage = 0;
            mRefreshTime = 0.0;
        }
    }

    return true;
}

void DemoWindow::Render()
{

}

void DemoWindow::UpdateCamera()
{
    Uint32 width, height;
    GetSize(width, height);

    const Camera oldCameraSetup = mCamera;

    rt::math::Vector4 direction = rt::math::Vector4(sinf(mCameraSetup.yaw) * cosf(mCameraSetup.pitch),
                                                  sinf(mCameraSetup.pitch),
                                                  cosf(mCameraSetup.yaw) * cosf(mCameraSetup.pitch));

    rt::math::Vector4 movement;
    if (IsKeyPressed('W'))
        movement += direction;
    if (IsKeyPressed('S'))
        movement -= direction;
    if (IsKeyPressed('A'))
        movement += rt::math::Vector4(-direction[2], 0.0f, direction[0]);
    if (IsKeyPressed('D'))
        movement -= rt::math::Vector4(-direction[2], 0.0f, direction[0]);

    if (movement.Length3() > RT_EPSILON)
    {
        movement.Normalize3();

        if (IsKeyPressed(VK_LSHIFT))
            movement *= 10.0f;
        else if (IsKeyPressed(VK_LCONTROL))
            movement /= 10.0f;

        mCameraSetup.position += movement * (Float)mDeltaTime;
    }

    mCamera.SetPerspective(mCameraSetup.position,
                           direction,
                           math::Vector4(0.0f, 1.0f, 0.0f),
                           (Float)width / (Float)height,
                           RT_PI / 180.0f * 60.0f);

    mCamera.Update();
}