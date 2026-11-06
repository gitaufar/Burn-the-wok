#pragma once

namespace engine {

// printf-style logging to stderr (visible in the console window of Debug builds).
void LogInfo(const char* format, ...);
void LogError(const char* format, ...);

// Logs the message and shows an OS message box. Use for errors that end the program,
// so players of a Release build (no console) still see what went wrong.
void ReportFatalError(const char* message);

} // namespace engine
