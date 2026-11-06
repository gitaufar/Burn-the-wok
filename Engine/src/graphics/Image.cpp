#include "engine/graphics/Image.h"

#include <algorithm>
#include <cstddef>

namespace engine {

Image::Image(int width, int height, Color fill)
    : m_width(std::max(width, 0)), m_height(std::max(height, 0)) {
    m_pixels.resize(static_cast<std::size_t>(m_width) * m_height * 4);
    FillRect(0, 0, m_width, m_height, fill);
}

void Image::SetPixel(int x, int y, Color color) {
    if (x < 0 || y < 0 || x >= m_width || y >= m_height) {
        return;
    }
    const std::size_t i = (static_cast<std::size_t>(y) * m_width + x) * 4;
    m_pixels[i + 0] = color.r;
    m_pixels[i + 1] = color.g;
    m_pixels[i + 2] = color.b;
    m_pixels[i + 3] = color.a;
}

Color Image::GetPixel(int x, int y) const {
    if (x < 0 || y < 0 || x >= m_width || y >= m_height) {
        return Color::Transparent();
    }
    const std::size_t i = (static_cast<std::size_t>(y) * m_width + x) * 4;
    return {m_pixels[i + 0], m_pixels[i + 1], m_pixels[i + 2], m_pixels[i + 3]};
}

void Image::FillRect(int x, int y, int width, int height, Color color) {
    const int x0 = std::max(x, 0);
    const int y0 = std::max(y, 0);
    const int x1 = std::min(x + width, m_width);
    const int y1 = std::min(y + height, m_height);
    for (int py = y0; py < y1; ++py) {
        for (int px = x0; px < x1; ++px) {
            SetPixel(px, py, color);
        }
    }
}

} // namespace engine
