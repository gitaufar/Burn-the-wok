#include "engine/graphics/SpriteBatch.h"

#include "engine/core/Log.h"
#include "engine/graphics/Image.h"
#include "engine/graphics/Shader.h"
#include "engine/graphics/Texture.h"
#include "GL.h"

#include <cstddef>

namespace engine {

namespace {

const char* const VertexShaderSource = R"(#version 330 core
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec4 aColor;

uniform mat4 uViewProjection;

out vec2 vTexCoord;
out vec4 vColor;

void main() {
    vTexCoord = aTexCoord;
    vColor = aColor;
    gl_Position = uViewProjection * vec4(aPosition, 0.0, 1.0);
}
)";

const char* const FragmentShaderSource = R"(#version 330 core
in vec2 vTexCoord;
in vec4 vColor;

uniform sampler2D uTexture;

out vec4 fragColor;

void main() {
    fragColor = texture(uTexture, vTexCoord) * vColor;
}
)";

} // namespace

SpriteBatch::SpriteBatch(int maxSpritesPerFlush) : m_maxSprites(maxSpritesPerFlush) {
    m_shader = std::make_unique<Shader>(VertexShaderSource, FragmentShaderSource);
    m_whiteTexture = std::make_unique<Texture>(Image(1, 1, Color::White()));
    m_vertices.reserve(static_cast<std::size_t>(m_maxSprites) * 4);

    // Every quad uses 4 vertices and 6 indices (two triangles: 0-1-2 and 2-3-0).
    // The index pattern never changes, so it is uploaded once.
    std::vector<GLuint> indices(static_cast<std::size_t>(m_maxSprites) * 6);
    for (int i = 0; i < m_maxSprites; ++i) {
        const GLuint base = static_cast<GLuint>(i * 4);
        GLuint* q = &indices[static_cast<std::size_t>(i) * 6];
        q[0] = base + 0;
        q[1] = base + 1;
        q[2] = base + 2;
        q[3] = base + 2;
        q[4] = base + 3;
        q[5] = base + 0;
    }

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(m_maxSprites * 4 * sizeof(Vertex)), nullptr,
                 GL_DYNAMIC_DRAW);

    glGenBuffers(1, &m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(GLuint)),
                 indices.data(), GL_STATIC_DRAW);

    const GLsizei stride = sizeof(Vertex);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<const void*>(offsetof(Vertex, x)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<const void*>(offsetof(Vertex, u)));
    // Colors are stored as 4 bytes and normalized to 0..1 by the GPU.
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride,
                          reinterpret_cast<const void*>(offsetof(Vertex, r)));

    glBindVertexArray(0);
}

SpriteBatch::~SpriteBatch() {
    glDeleteBuffers(1, &m_ibo);
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
}

void SpriteBatch::Begin(const Mat4& viewProjection) {
    if (m_drawing) {
        LogError("SpriteBatch::Begin called twice without End");
        return;
    }
    m_drawing = true;
    m_drawCalls = 0;
    m_currentTexture = nullptr;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_shader->Use();
    m_shader->SetMat4("uViewProjection", viewProjection);
    m_shader->SetInt("uTexture", 0);
}

void SpriteBatch::End() {
    if (!m_drawing) {
        LogError("SpriteBatch::End called without Begin");
        return;
    }
    Flush();
    m_drawing = false;
    m_currentTexture = nullptr;
}

void SpriteBatch::Draw(const Texture& texture, const Rect& dest, const Rect& source, Color tint) {
    if (!m_drawing) {
        LogError("SpriteBatch::Draw called outside Begin/End");
        return;
    }

    // A texture switch breaks the batch: send what we have so far, then continue.
    if (m_currentTexture != &texture) {
        Flush();
        m_currentTexture = &texture;
    }
    if (static_cast<int>(m_vertices.size()) >= m_maxSprites * 4) {
        Flush();
    }

    const float invW = 1.0f / static_cast<float>(texture.GetWidth());
    const float invH = 1.0f / static_cast<float>(texture.GetHeight());
    const float u0 = source.x * invW;
    const float v0 = source.y * invH;
    const float u1 = (source.x + source.w) * invW;
    const float v1 = (source.y + source.h) * invH;

    const float x0 = dest.x;
    const float y0 = dest.y;
    const float x1 = dest.x + dest.w;
    const float y1 = dest.y + dest.h;

    // Order: top-left, top-right, bottom-right, bottom-left (matches the index pattern).
    m_vertices.push_back({x0, y0, u0, v0, tint.r, tint.g, tint.b, tint.a});
    m_vertices.push_back({x1, y0, u1, v0, tint.r, tint.g, tint.b, tint.a});
    m_vertices.push_back({x1, y1, u1, v1, tint.r, tint.g, tint.b, tint.a});
    m_vertices.push_back({x0, y1, u0, v1, tint.r, tint.g, tint.b, tint.a});
}

void SpriteBatch::Draw(const Texture& texture, const Rect& dest, Color tint) {
    const Rect whole{0.0f, 0.0f, static_cast<float>(texture.GetWidth()),
                     static_cast<float>(texture.GetHeight())};
    Draw(texture, dest, whole, tint);
}

void SpriteBatch::DrawRect(const Rect& dest, Color color) {
    Draw(*m_whiteTexture, dest, Rect{0.0f, 0.0f, 1.0f, 1.0f}, color);
}

void SpriteBatch::DrawRectOutline(const Rect& dest, float thickness, Color color) {
    const float t = thickness;
    DrawRect({dest.x, dest.y, dest.w, t}, color);                      // top
    DrawRect({dest.x, dest.y + dest.h - t, dest.w, t}, color);         // bottom
    DrawRect({dest.x, dest.y + t, t, dest.h - 2.0f * t}, color);       // left
    DrawRect({dest.x + dest.w - t, dest.y + t, t, dest.h - 2.0f * t}, color); // right
}

void SpriteBatch::Flush() {
    if (m_vertices.empty() || !m_currentTexture) {
        return;
    }

    m_currentTexture->Bind(0);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(m_vertices.size() * sizeof(Vertex)),
                    m_vertices.data());

    const GLsizei indexCount = static_cast<GLsizei>(m_vertices.size() / 4 * 6);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    m_vertices.clear();
    ++m_drawCalls;
}

} // namespace engine
