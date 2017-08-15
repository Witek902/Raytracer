#include "PCH.h"
#include "Demo.h"


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    RT_UNUSED(hPrevInstance);
    RT_UNUSED(lpCmdLine);

    {
        auto instance = rt::Instance::CreateCpuInstance();
        if (!instance)
        {
            return 1;
        }

        DemoWindow demo(*instance);

        if (!demo.Initialize())
        {
            return 2;
        }

        if (!demo.Loop())
        {
            return 3;
        }
    }

#ifdef _DEBUG
    _CrtDumpMemoryLeaks();
#endif

    return 0;
}
