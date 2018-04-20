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
            return 2;
        }

        if (!demo.InitScene())
        {
            return 3;
        }

        if (!demo.Loop())
        {
            return 4;
        }
    }

#ifdef _DEBUG
    _CrtDumpMemoryLeaks();
#endif

    return 0;
}
