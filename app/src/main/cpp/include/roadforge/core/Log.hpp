#pragma once

#include <android/log.h>

namespace roadforge::core {

inline constexpr const char* kLogTag = "RoadForge";

void logPrint(android_LogPriority priority, const char* file, int line, const char* fmt, ...);

} // namespace roadforge::core

#define RF_LOGD(...) ::roadforge::core::logPrint(ANDROID_LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define RF_LOGI(...) ::roadforge::core::logPrint(ANDROID_LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define RF_LOGW(...) ::roadforge::core::logPrint(ANDROID_LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define RF_LOGE(...) ::roadforge::core::logPrint(ANDROID_LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
