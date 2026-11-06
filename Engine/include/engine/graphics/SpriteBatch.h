#pragma once

#include "engine/graphics/Color.h"
#include "engine/math/Mat4.h"
#include "engine/math/Rect.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace engine {

class Shader;
class Texture;

// Collects textured quads and sends them to the GPU in as few draw calls as
// possible: one draw call per run of consecutive sprites that share a texture.
// Draw order = call order (later calls are drawn on top). Putting many sprites
// in one atlas texture therefore keeps the draw-call count low.
//
// Usage per frame:
//   batch.Begin(camera.GetViewProjection());
//   batch.Draw(...); batch.Draw(...);
//   batch.End();
class SpriteBatch {
public:
    explicit SpriteBatch(int maxSpritesPerFlush = 4096);
    ~SpriteBatch();

    SpriteBatch(const SpriteBatch&) = delete;
    SpriteBatch& operator=(const SpriteBatch&) = delete;

    void Begin(const Mat4& viewProjection);
    void End();

    // dest is in world units, source is in texture pixels (top-left origin).
    void Draw(const Texture& texture, const Rect& dest, const Rect& source, Color tint = Color::White());
    // Draws the whole texture.
    void Draw(const Texture& texture, const Rect& dest, Color tint = Color::White());

    // Solid-color shapes, drawn with a built-in 1x1 white texture.
    void DrawRect(const Rect& dest, Color color);
    void DrawRectOutline(const Rect& dest, float thickness, Color color);

    // Number of GPU draw calls issued since the last Begin(). Useful for debugging batching.
    int GetDrawCallCount() const { return m_drawCalls; }

private:
    struct Vertex {
        float x, y;
        float u, v;
        std::uint8_t r, g, b, a;
    };

    void Flush();

    std::unique_ptr<Shader> m_shader;
    std::unique_ptr<Texture> m_whiteTexture;
    std::vector<Vertex> m_vertices;
    const Texture* m_currentTexture = nullptr;
    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    unsigned int m_ibo = 0;
    int m_maxSprites = 0;
    int m_drawCalls = 0;
    bool m_drawing = false;
};

} // namespace engine
