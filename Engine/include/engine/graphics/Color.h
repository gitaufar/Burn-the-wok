#pragma once

#include <cstdint>

namespace engine {

// 8-bit RGBA color, the same byte layout as texture pixels.
struct Color {
    std::uint8_t r = 255;
    std::uint8_t g = 255;
    std::uint8_t b = 255;
    std::uint8_t a = 255;

    // Color::FromHex(0xFF8800) -> orange. Handy for copying values from a pixel-art palette.
    static constexpr Color FromHex(std::uint32_t rgb, std::uint8_t alpha = 255) {
        return {static_cast<std::uint8_t>((rgb >> 16) & 0xFF),
                static_cast<std::uint8_t>((rgb >> 8) & 0xFF),
                static_cast<std::uint8_t>(rgb & 0xFF),
                alpha};
    }

    constexpr Color WithAlpha(std::uint8_t alpha) const { return {r, g, b, alpha}; }

    static constexpr Color White() { return {255, 255, 255, 255}; }
    static constexpr Color Black() { return {0, 0, 0, 255}; }
    static constexpr Color Transparent() { return {0, 0, 0, 0}; }

    constexpr bool operator==(const Color&) const = default;
};

} // namespace engine
