#include "engine/core/AssetPath.h"

#include <SDL.h>

#include <memory>

namespace engine {
std::filesystem::path ResolveAssetPath(const std::filesystem::path& path) {
    if (path.is_absolute()) return path;
    const std::unique_ptr<char, decltype(&SDL_free)> base(SDL_GetBasePath(), &SDL_free);
    if (base) {
        const auto resolved = std::filesystem::path(base.get()) / path;
        if (std::filesystem::exists(resolved)) return resolved;
    }
    return path;
}
} // namespace engine
