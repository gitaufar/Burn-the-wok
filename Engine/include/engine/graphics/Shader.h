#pragma once

namespace engine {

struct Mat4;

// Linked GLSL program (vertex + fragment). Throws std::runtime_error with the
// compiler log when compilation or linking fails.
class Shader {
public:
    Shader(const char* vertexSource, const char* fragmentSource);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    void Use() const;
    void SetMat4(const char* name, const Mat4& value) const;
    void SetInt(const char* name, int value) const;

private:
    unsigned int m_program = 0; // GLuint
};

} // namespace engine
