#include "engine/graphics/Shader.h"

#include "engine/math/Mat4.h"
#include "GL.h"

#include <stdexcept>
#include <string>

namespace engine {

namespace {

GLuint CompileStage(GLenum stage, const char* source) {
    const GLuint shader = glCreateShader(stage);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (ok != GL_TRUE) {
        char log[1024] = {};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        glDeleteShader(shader);
        const char* name = stage == GL_VERTEX_SHADER ? "vertex" : "fragment";
        throw std::runtime_error(std::string("Shader compile error (") + name + "):\n" + log);
    }
    return shader;
}

} // namespace

Shader::Shader(const char* vertexSource, const char* fragmentSource) {
    const GLuint vs = CompileStage(GL_VERTEX_SHADER, vertexSource);
    GLuint fs = 0;
    try {
        fs = CompileStage(GL_FRAGMENT_SHADER, fragmentSource);
    } catch (...) {
        glDeleteShader(vs);
        throw;
    }

    m_program = glCreateProgram();
    glAttachShader(m_program, vs);
    glAttachShader(m_program, fs);
    glLinkProgram(m_program);

    // The program keeps the compiled code; the stage objects are no longer needed.
    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint ok = GL_FALSE;
    glGetProgramiv(m_program, GL_LINK_STATUS, &ok);
    if (ok != GL_TRUE) {
        char log[1024] = {};
        glGetProgramInfoLog(m_program, sizeof(log), nullptr, log);
        glDeleteProgram(m_program);
        m_program = 0;
        throw std::runtime_error(std::string("Shader link error:\n") + log);
    }
}

Shader::~Shader() {
    if (m_program != 0) {
        glDeleteProgram(m_program);
    }
}

void Shader::Use() const {
    glUseProgram(m_program);
}

void Shader::SetMat4(const char* name, const Mat4& value) const {
    glUniformMatrix4fv(glGetUniformLocation(m_program, name), 1, GL_FALSE, value.Data());
}

void Shader::SetInt(const char* name, int value) const {
    glUniform1i(glGetUniformLocation(m_program, name), value);
}

} // namespace engine
