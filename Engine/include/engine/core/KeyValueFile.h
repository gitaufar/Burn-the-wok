#pragma once

#include <filesystem>
#include <map>
#include <string>

namespace engine {
class KeyValueFile {
public:
    using Values = std::map<std::string, std::string>;
    static bool Read(const std::filesystem::path& path, Values& values);
    static bool Write(const std::filesystem::path& path, const Values& values);
};
}
