#include "PCH.h"
#include "Demo.h"


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    RT_UNUSED(hPrevInstance);
    RT_UNUSED(lpCmdLine);

    {
        DemoWindow demo;

        if (!demo.Initialize())
        {
            return 1;
        }

        if (!demo.Loop())
        {
            return 2;
        }
    }

#ifdef _DEBUG
    _CrtDumpMemoryLeaks();
#endif

    return 0;
}


/*
void Demo_UpdateCamera()
{
    if (!g_MouseInfo.LeftButton)
        return;

    float Velocity;
    rtVector Movement;

    if (g_Keys[VK_SHIFT])
        Velocity = 25.0f;
    else
    {
        if (g_Keys[VK_CONTROL])
            Velocity = 0.2f;
        else
            Velocity = 2.0f;
    }


    if (g_MouseInfo.LeftButton)
    {
        g_Camera.XZAngle = g_Camera.XZAngleOld - 0.005f * (float)(g_MouseInfo.PX - g_MouseInfo.X);
        g_Camera.YAngle = g_Camera.YAngleOld - 0.005f * (float)(g_MouseInfo.Y - g_MouseInfo.PY);

        if (g_Camera.YAngle<-1.57f)
            g_Camera.YAngle = -1.57f;
        if (g_Camera.YAngle>1.57f)
            g_Camera.YAngle = 1.57f;

        if (g_Camera.XZAngle>3.14159f*2.0f)
            g_Camera.XZAngle -= 3.14159f*2.0f;
        if (g_Camera.XZAngle<0.0f)
            g_Camera.XZAngle += 3.14159f*2.0f;
    }

    g_Camera.Dir = rtVectorMake(sinf(g_Camera.XZAngle)*cosf(g_Camera.YAngle), sinf(g_Camera.YAngle),
                                cosf(g_Camera.XZAngle)*cosf(g_Camera.YAngle), 0.0f);
    g_Camera.Dir = rtVectorNormalize3(g_Camera.Dir);

    Movement = rtVectorZero();

    if (g_Keys['W'])
        Movement += g_Camera.Dir;
    if (g_Keys['S'])
        Movement -= g_Camera.Dir;
    if (g_Keys['A'])
        Movement += rtVectorMake(-rtVectorGet(g_Camera.Dir, 2), 0.0f, rtVectorGet(g_Camera.Dir, 0), 0.0f);
    if (g_Keys['D'])
        Movement += rtVectorMake(rtVectorGet(g_Camera.Dir, 2), 0.0f, -rtVectorGet(g_Camera.Dir, 0), 0.0f);

    if (rtVectorGreater(rtVectorLength3(Movement), g_XMEpsilon))
    {
        Movement = rtVectorNormalize3(Movement);
        Movement *= Velocity;
        Movement *= (float)g_DeltaTime;

        g_Camera.Pos += Movement;
    }
}
*/