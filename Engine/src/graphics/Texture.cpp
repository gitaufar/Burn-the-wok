#include "engine/graphics/Texture.h"

#include "engine/graphics/Image.h"
#include "GL.h"

#include <stdexcept>
#include <utility>

namespace engine {

Texture::Texture(const Image& image) : m_width(image.GetWidth()), m_height(image.GetHeight()) {
    if (m_width <= 0 || m_height <= 0) {
        throw std::runtime_error("Texture: image is empty");
    }

    glGenTextures(1, &m_handle);
    glBindTexture(GL_TEXTURE_2D, m_handle);

    // NEAREST = pick the closest texel, no blending between texels: crisp pixel art.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Rows are tightly packed RGBA8. Image row 0 is the top row, and SpriteBatch computes
    // texture coordinates from the top as well, so no vertical flip is needed.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 image.GetData());

    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture() {
    if (m_handle != 0) {
        glDeleteTextures(1, &m_handle);
    }
}

Texture::Texture(Texture&& other) noexcept
    : m_handle(std::exchange(other.m_handle, 0)),
      m_width(std::exchange(other.m_width, 0)),
      m_height(std::exchange(other.m_height, 0)) {}

Texture& Texture::operator=(Texture&& other) noexcept {
    if (this != &other) {
        if (m_handle != 0) {
            glDeleteTextures(1, &m_handle);
        }
        m_handle = std::exchange(other.m_handle, 0);
        m_width = std::exchange(other.m_width, 0);
        m_height = std::exchange(other.m_height, 0);
    }
    return *this;
}

void Texture::Bind(int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_handle);
}

} // namespace engine
