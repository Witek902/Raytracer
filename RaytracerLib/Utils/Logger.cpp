#include "PCH.h"
#include "Logger.h"

#include <iostream>


namespace rt {

static std::mutex gLogMutex;

void LogGeneric(LogType type, const char* str, ...)
{
    const int SHORT_MESSAGE_LENGTH = 4096;
    char buffer[SHORT_MESSAGE_LENGTH];

    if (str == nullptr)
        return;

    va_list args, argsCopy;
    va_start(args, str);

    // we can't call vsnprintf with the same va_list more than once
    va_copy(argsCopy, args);

    int len = vsnprintf(buffer, SHORT_MESSAGE_LENGTH - 1, str, args);
    if (len < 0)
    {
        va_end(argsCopy);
        va_end(args);
        RT_LOG_ERROR("vsnprintf() failed", str);
        return;
    }

    va_end(argsCopy);
    va_end(args);

    const char* logTypeStr = "";
    switch (type)
    {
    case LogType::Debug:
        logTypeStr = "[DEBUG] ";
        break;
    case LogType::Info:
        logTypeStr = "[INFO] ";
        break;
    case LogType::Warning:
        logTypeStr = "[WARNING] ";
        break;
    case LogType::Error:
        logTypeStr = "[ERROR] ";
        break;
    }

    {
        std::lock_guard<std::mutex> lock(gLogMutex);

        std::cout << logTypeStr << buffer << std::endl;
#ifdef WIN32
        OutputDebugStringA(logTypeStr);
        OutputDebugStringA(buffer);
        OutputDebugStringA("\n");
#endif // WIN32
    }
}

} // namespace rt
