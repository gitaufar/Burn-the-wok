#include "engine/core/Log.h"

#include <SDL/SDL.h>

#include <cstdarg>
#include <cstdio>

namespace engine {

namespace {

void LogWithPrefix(const char* prefix, const char* format, va_list args) {
    std::fprintf(stderr, "%s", prefix);
    std::vfprintf(stderr, format, args);
    std::fprintf(stderr, "\n");
}

} // namespace

void LogInfo(const char* format, ...) {
    va_list args;
    va_start(args, format);
    LogWithPrefix("[info]  ", format, args);
    va_end(args);
}

void LogError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    LogWithPrefix("[error] ", format, args);
    va_end(args);
}

void ReportFatalError(const char* message) {
    LogError("%s", message);
    // Works even if SDL_Init failed or was never called.
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", message, nullptr);
}

} // namespace engine
