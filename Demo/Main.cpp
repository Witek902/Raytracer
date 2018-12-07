#include "PCH.h"
#include "Demo.h"

#include "../External/cxxopts.hpp"
#include "../Core/Utils/Logger.h"

bool ParseOptions(int argc, char** argv, Options& outOptions)
{
    cxxopts::Options options("Raytracer Demo", "CPU raytracer by Michal Witanowski");
    options.add_options()
        ("w,width", "Window width", cxxopts::value<Uint32>())
        ("h,height", "Window width", cxxopts::value<Uint32>())
        ("s,scene", "Initial scene", cxxopts::value<std::string>())
        ("debug-renderer", "Use debug renderer by default", cxxopts::value<bool>())
        ("p,packet-tracing", "Use ray packet tracing by default", cxxopts::value<bool>())
        ("data", "Data path", cxxopts::value<std::string>())
        ;

    try
    {
        auto result = options.parse(argc, argv);

        if (result.count("w"))
            outOptions.windowWidth = result["w"].as<Uint32>();

        if (result.count("h"))
            outOptions.windowHeight = result["h"].as<Uint32>();

        if (result.count("data"))
            outOptions.dataPath = result["data"].as<std::string>();

        if (result.count("scene"))
            outOptions.sceneName = result["scene"].as<std::string>();

        outOptions.useDebugRenderer = result["debug-renderer"].count() > 0;

        outOptions.enablePacketTracing = result["p"].count() > 0;
    }
    catch (cxxopts::OptionParseException& e)
    {
        RT_LOG_ERROR("Failed to parse commandline: %hs", e.what());
        return false;
    }

    return true;
}

Options gOptions;

int main(int argc, char* argv[])
{
    rt::math::SetFlushDenormalsToZero();

    if (!ParseOptions(argc, argv, gOptions))
    {
        return 1;
    }

    {
        DemoWindow demo;

        if (!demo.Initialize())
        {
            return 2;
        }

        RT_LOG_INFO("Initialized.");

        if (!demo.Loop())
        {
            return 3;
        }
    }

    RT_LOG_INFO("Closing.");

    RT_ASSERT(rt::math::GetFlushDenormalsToZero(), "Something disabled flushing denormal float to zero");

#if defined(_DEBUG) && defined(WIN32)
    _CrtDumpMemoryLeaks();
#endif // _DEBUG

    return 0;
}
