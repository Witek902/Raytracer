#pragma once

#include "../RayLib.h"


namespace rt {

enum class LogType
{
    Debug,
    Info,
    Warning,
    Error,
};

void RAYLIB_API LogGeneric(LogType type, const char* str, ...);

} // namespace rt


// TODO
#define RT_LOG_DEBUG(...)      rt::LogGeneric(rt::LogType::Debug, __VA_ARGS__)
#define RT_LOG_INFO(...)       rt::LogGeneric(rt::LogType::Info, __VA_ARGS__)
#define RT_LOG_WARNING(...)    rt::LogGeneric(rt::LogType::Warning, __VA_ARGS__)
#define RT_LOG_ERROR(...)      rt::LogGeneric(rt::LogType::Error, __VA_ARGS__)
