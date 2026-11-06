#include "engine/graphics/Window.h"

#include "engine/core/Log.h"
#include "GL.h"

#include <SDL/SDL.h>

#include <stdexcept>

namespace engine {

Window::Window(const std::string& title, int width, int height, bool vsync, bool resizable) {
    // Request a modern (3.3 core) context: shaders + vertex buffers, no fixed-function pipeline.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    Uint32 flags = SDL_WINDOW_OPENGL;
    if (resizable) {
        flags |= SDL_WINDOW_RESIZABLE;
    }

    m_window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                width, height, flags);
    if (!m_window) {
        throw std::runtime_error(std::string("SDL_CreateWindow failed: ") + SDL_GetError());
    }

    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext) {
        const std::string error = std::string("OpenGL 3.3 context creation failed: ") + SDL_GetError();
        Destroy();
        throw std::runtime_error(error);
    }

    // 1 = wait for the monitor's vertical refresh (no tearing, no 100% CPU spin).
    SDL_GL_SetSwapInterval(vsync ? 1 : 0);

    // Core profiles need this or GLEW skips functions it cannot find via the extension list.
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        Destroy();
        throw std::runtime_error("glewInit failed");
    }
    // glewInit can leave a harmless GL_INVALID_ENUM behind on core profiles; clear it.
    glGetError();

    LogInfo("OpenGL %s | %s", reinterpret_cast<const char*>(glGetString(GL_VERSION)),
            reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

    HandleResize();
}

Window::~Window() {
    Destroy();
}

void Window::Destroy() {
    if (m_glContext) {
        SDL_GL_DeleteContext(m_glContext);
        m_glContext = nullptr;
    }
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
}

void Window::SetTitle(const std::string& title) {
    SDL_SetWindowTitle(m_window, title.c_str());
}

void Window::HandleResize() {
    SDL_GL_GetDrawableSize(m_window, &m_width, &m_height);
}

void Window::BeginFrame() {
    glViewport(0, 0, m_width, m_height);
    glClearColor(m_clearColor.r / 255.0f, m_clearColor.g / 255.0f, m_clearColor.b / 255.0f,
                 m_clearColor.a / 255.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Window::Present() {
    SDL_GL_SwapWindow(m_window);
}

} // namespace engine
