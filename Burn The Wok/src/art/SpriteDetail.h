#pragma once

#include <engine/graphics/Image.h>

#include <algorithm>

namespace game::art {

// Scale2x refines diagonal pixel-art edges using only existing palette colors.
// Prepare once on load; nearest texture filtering keeps the result crisp.
inline engine::Image BuildDetailedSprite(const engine::Image& source) {
    engine::Image result(source.GetWidth() * 2, source.GetHeight() * 2);
    auto sample = [&](int x, int y) {
        auto color = source.GetPixel(std::clamp(x, 0, source.GetWidth() - 1),
                                     std::clamp(y, 0, source.GetHeight() - 1));
        // Invisible RGB values should not affect silhouette comparisons.
        return color.a == 0 ? engine::Color::Transparent() : color;
    };
    for (int y = 0; y < source.GetHeight(); ++y) {
        for (int x = 0; x < source.GetWidth(); ++x) {
            const auto b = sample(x, y - 1), d = sample(x - 1, y);
            const auto e = sample(x, y), f = sample(x + 1, y);
            const auto h = sample(x, y + 1);
            const bool refine = b != h && d != f;
            result.SetPixel(x * 2, y * 2, refine && d == b ? d : e);
            result.SetPixel(x * 2 + 1, y * 2, refine && b == f ? f : e);
            result.SetPixel(x * 2, y * 2 + 1, refine && d == h ? d : e);
            result.SetPixel(x * 2 + 1, y * 2 + 1, refine && h == f ? f : e);
        }
    }
    return result;
}

} // namespace game::art
