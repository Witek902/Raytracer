#pragma once

#include "Window.h"
#include "../RaytracerLib/Random.h"


class DemoWindow : public Window
{
public:
    DemoWindow();

    bool Initialize();

    /**
     * Main loop.
     */
    bool Loop();

    /**
     * Reset counters.
     */
    void Reset();

    void Render();

private:
    Random mRandomGenerator;

    // TODO temporary, will be managed by "Bitmap" class
    std::unique_ptr<Uint32[]> mFrameBuffer;

    Uint32 mFrameNumber;
    Uint32 mFrameCounterForAverage;

    Double mDeltaTime;
    Double mTotalTime;
    Double mRefreshTime;

    void OnKeyPress(Uint32 key) override;
    void OnResize(Uint32 width, Uint32 height) override;
};