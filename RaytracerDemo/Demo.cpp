#include "PCH.h"
#include "Demo.h"
#include "Timer.h"
#include "../RaytracerLib/Logger.h"


using namespace rt;

namespace {

Uint32 WINDOW_WIDTH = 1280;
Uint32 WINDOW_HEIGHT = 720;

}

DemoWindow::DemoWindow()
    : mFrameNumber(0)
    , mDeltaTime(0.0)
    , mTotalTime(0.0)
    , mRefreshTime(0.0)
{
    Reset();
}

bool DemoWindow::Initialize()
{
    if (!Init())
    {
        LOG_ERROR("Failed to init window");
        return false;
    }

    SetSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    SetTitle("Raytracer Demo");

    if (!Open())
    {
        LOG_ERROR("Failed to open window");
        return false;
    }

    return true;
}

void DemoWindow::Reset()
{
    mFrameNumber = 0;
    mFrameCounterForAverage = 0;

    mDeltaTime = 0.0;
    mTotalTime = 0.0;
    mRefreshTime = 0.0;
}

void DemoWindow::OnResize(Uint32 width, Uint32 height)
{
    mFramebuffer.Init(width, height, Bitmap::Format::R8G8B8A8_Uint);

    UpdateCamera();
}

bool DemoWindow::InitScene()
{
    mScene.reset(new rt::Scene);



    return true;
}

void DemoWindow::OnKeyPress(Uint32 key)
{
    if (key == VK_F1)
    {
        SetFullscreenMode(!GetFullscreenMode());
    }
}

bool DemoWindow::Loop()
{
    Timer timer;
    timer.Start();

    while (!IsClosed())
    {
        ProcessMessages();

        // update time & frame stats
        const double dt = timer.Reset();
        mDeltaTime = dt;
        mTotalTime += dt;
        mRefreshTime += dt;
        mFrameNumber++;
        mFrameCounterForAverage++;

        Render();

        // refresh window title bar
        if (mRefreshTime > 1.0)
        {
            const double avgDt = mRefreshTime / static_cast<Double>(mFrameCounterForAverage);

            std::stringstream stringStream;
            stringStream << "Raytracer Demo " << "dt = " << (1000.0 * avgDt) << "ms, " << "frame " << mFrameNumber;
            SetTitle(stringStream.str().c_str());

            mFrameCounterForAverage = 0;
            mRefreshTime = 0.0;
        }
    }

    return true;
}

void DemoWindow::Render()
{
    Uint32 width, height;
    GetSize(width, height);

    RaytracingParams params;
    mScene->Raytrace(mCamera, mFramebuffer, params);

    Paint(width, height, mFramebuffer.GetData());
}

void DemoWindow::UpdateCamera()
{
    Uint32 width, height;
    GetSize(width, height);

    mCamera.SetPerspective(math::Vector(0.0f, 0.0f, -5.0f),
                           math::Vector(0.0f, 0.0f, 1.0f),
                           math::Vector(0.0f, 1.0f, 0.0f),
                           (Float)width / (Float)height,
                           RT_PI * 70.0f / 180.0f);

    mCamera.Update();
}