#pragma once

#include <filesystem>

namespace engine {
// Prefer assets beside the executable; retain working-directory fallback for development.
std::filesystem::path ResolveAssetPath(const std::filesystem::path& path);
} // namespace engine
