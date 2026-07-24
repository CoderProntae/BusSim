#include "roadforge/core/Log.hpp"

#include <cstdarg>
#include <cstdio>

namespace roadforge::core {

void logPrint(android_LogPriority priority, const char* file, int line, const char* fmt, ...) {
    char message[1024];

    va_list args;
    va_start(args, fmt);
    std::vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);

    const char* base = file;
    for (const char* p = file; *p != '\0'; ++p) {
        if (*p == '/' || *p == '\\') {
            base = p + 1;
        }
    }

    __android_log_print(priority, kLogTag, "%s:%d | %s", base, line, message);
}

} // namespace roadforge::core
