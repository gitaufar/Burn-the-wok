#include <engine/core/KeyValueFile.h>

#include <atomic>
#include <fstream>
#include <utility>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace engine {
bool KeyValueFile::Read(const std::filesystem::path& path, Values& values) {
    std::ifstream stream(path);
    if (!stream) return false;
    Values parsed;
    std::string line;
    while (std::getline(stream, line)) {
        const auto equal = line.find('=');
        if (equal == std::string::npos || equal == 0 ||
            !parsed.emplace(line.substr(0, equal), line.substr(equal+1)).second) return false;
    }
    if (!stream.eof() || stream.bad() || parsed.empty()) return false;
    values = std::move(parsed);
    return true;
}

bool KeyValueFile::Write(const std::filesystem::path& path, const Values& values) {
    if (values.empty()) return false;
    for (const auto& [key, value] : values) {
        if (key.empty() || key.find_first_of("=\r\n") != std::string::npos ||
            value.find_first_of("\r\n") != std::string::npos) return false;
    }
    static std::atomic<unsigned> sequence{0};
    auto temporary = path;
#ifdef _WIN32
    const auto process = GetCurrentProcessId();
#else
    const auto process = getpid();
#endif
    temporary += ".tmp." + std::to_string(process) + "." +
                 std::to_string(sequence.fetch_add(1, std::memory_order_relaxed));
    std::ofstream stream(temporary, std::ios::trunc);
    if (!stream) return false;
    for (const auto& [key, value] : values) stream << key << '=' << value << '\n';
    stream.close();
    std::error_code error;
    if (stream) {
#ifdef _WIN32
        if (MoveFileExW(temporary.c_str(), path.c_str(),
                        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) return true;
#else
        std::filesystem::rename(temporary, path, error);
        if (!error) return true;
#endif
    }
    std::filesystem::remove(temporary, error);
    return false;
}
}
