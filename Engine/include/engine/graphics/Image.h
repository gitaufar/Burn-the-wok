#pragma once

#include "engine/graphics/Color.h"

#include <cstdint>
#include <vector>

namespace engine {

// CPU-side RGBA8 pixel buffer. Row 0 is the TOP row of the image.
// Textures are created from an Image; a file loader (PNG, ...) will later
// decode into an Image too, so the GPU side never cares where pixels came from.
class Image {
public:
    Image() = default;
    Image(int width, int height, Color fill = Color::Transparent());

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    const std::uint8_t* GetData() const { return m_pixels.data(); }

    // Out-of-range coordinates are ignored, which keeps procedural drawing code simple.
    void SetPixel(int x, int y, Color color);
    Color GetPixel(int x, int y) const;
    void FillRect(int x, int y, int width, int height, Color color);

private:
    int m_width = 0;
    int m_height = 0;
    std::vector<std::uint8_t> m_pixels; // 4 bytes per pixel: R, G, B, A
};

} // namespace engine
