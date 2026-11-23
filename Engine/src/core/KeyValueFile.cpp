#include <engine/core/KeyValueFile.h>

#include <atomic>
#include <fstream>
#include <utility>
#include <windows.h>

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
    temporary += L".tmp." + std::to_wstring(GetCurrentProcessId()) + L"." +
                 std::to_wstring(sequence.fetch_add(1, std::memory_order_relaxed));
    std::ofstream stream(temporary, std::ios::trunc);
    if (!stream) return false;
    for (const auto& [key, value] : values) stream << key << '=' << value << '\n';
    stream.close();
    if (stream && MoveFileExW(temporary.c_str(), path.c_str(),
                              MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) return true;
    std::error_code error;
    std::filesystem::remove(temporary, error);
    return false;
}
}
