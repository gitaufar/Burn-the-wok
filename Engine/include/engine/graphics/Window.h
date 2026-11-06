#pragma once

#include "engine/graphics/Color.h"
#include "engine/math/Vec2.h"

#include <string>

struct SDL_Window;

namespace engine {

// OS window plus its OpenGL 3.3 core context (created through SDL2, functions loaded with GLEW).
// SDL video must already be initialized; Application takes care of that.
class Window {
public:
    Window(const std::string& title, int width, int height, bool vsync, bool resizable);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    void SetTitle(const std::string& title);

    // Size of the drawable area in pixels.
    Vec2 GetSize() const { return {static_cast<float>(m_width), static_cast<float>(m_height)}; }

    void SetClearColor(Color color) { m_clearColor = color; }

    void BeginFrame(); // sets the viewport and clears the screen
    void Present();    // swaps front/back buffers

    void HandleResize(); // re-reads the drawable size after the OS resized the window

private:
    void Destroy();

    SDL_Window* m_window = nullptr;
    void* m_glContext = nullptr; // SDL_GLContext is a typedef for void*
    int m_width = 0;
    int m_height = 0;
    Color m_clearColor = Color::Black();
};

} // namespace engine
