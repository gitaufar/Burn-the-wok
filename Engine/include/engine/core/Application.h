#pragma once

#include "engine/audio/Audio.h"
#include "engine/core/SceneManager.h"
#include "engine/graphics/SpriteBatch.h"
#include "engine/graphics/Window.h"
#include "engine/input/Input.h"

#include <memory>
#include <string>

namespace engine {

struct AppConfig {
    std::string title = "Engine";
    int width = 1280;
    int height = 720;
    int updatesPerSecond = 60; // fixed update rate
    bool vsync = true;
    bool resizable = true;
};

// Owns every engine subsystem and runs the game loop.
//
// Game loop (fixed timestep, "Fix Your Timestep" by Glenn Fiedler):
//   - real elapsed time is added to an accumulator,
//   - the game is updated in fixed steps of 1/updatesPerSecond while the accumulator allows,
//   - then one frame is rendered with alpha = leftover / step for interpolation.
// Game logic therefore behaves the same on a 30 Hz laptop and a 144 Hz monitor.
class Application {
public:
    explicit Application(const AppConfig& config);
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    // Runs until Quit() is called, the window is closed, or the scene stack becomes empty.
    int Run(std::unique_ptr<Scene> firstScene);
    void Quit() { m_running = false; }

    Window& GetWindow() { return *m_window; }
    SpriteBatch& GetSpriteBatch() { return *m_spriteBatch; }
    SceneManager& GetScenes() { return *m_scenes; }
    AudioSystem& GetAudio() { return *m_audio; }
    const Input& GetInput() const { return m_input; }
    float GetFixedDeltaTime() const { return 1.0f / static_cast<float>(m_config.updatesPerSecond); }

private:
    void PollEvents();

    AppConfig m_config;
    // Declaration order matters for cleanup: the destructor tears these down
    // explicitly in reverse order (scenes -> sprite batch -> window -> SDL).
    std::unique_ptr<Window> m_window;
    std::unique_ptr<SpriteBatch> m_spriteBatch;
    std::unique_ptr<AudioSystem> m_audio; // never throws: stays silent without an audio device
    std::unique_ptr<SceneManager> m_scenes;
    Input m_input;
    bool m_running = false;
};

} // namespace engine
