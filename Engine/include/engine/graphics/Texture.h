#pragma once

namespace engine {

class Image;

// GPU texture (OpenGL). Always uses NEAREST filtering so pixel art stays sharp.
// Owns the GL handle: movable, not copyable. Requires a live GL context.
class Texture {
public:
    explicit Texture(const Image& image);
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

    void Bind(int unit = 0) const;

private:
    unsigned int m_handle = 0; // GLuint
    int m_width = 0;
    int m_height = 0;
};

} // namespace engine
