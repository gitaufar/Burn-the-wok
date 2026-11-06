#include "engine/core/Application.h"

#include "engine/core/Log.h"

#include <SDL/SDL.h>

#include <stdexcept>
#include <string>
#include <utility>

namespace engine {

Application::Application(const AppConfig& config) : m_config(config) {
    if (m_config.updatesPerSecond <= 0) {
        m_config.updatesPerSecond = 60;
    }

    // We define SDL_MAIN_HANDLED and use a normal main(), so tell SDL that
    // the program entry point has already been set up.
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());
    }

    try {
        m_window = std::make_unique<Window>(m_config.title, m_config.width, m_config.height,
                                            m_config.vsync, m_config.resizable);
        m_spriteBatch = std::make_unique<SpriteBatch>();
        m_scenes = std::make_unique<SceneManager>(*this);
    } catch (...) {
        // The destructor does not run when a constructor throws, so clean up here.
        m_spriteBatch.reset();
        m_window.reset();
        SDL_Quit();
        throw;
    }
}

Application::~Application() {
    // Scenes own textures and the sprite batch owns GL buffers: both must be
    // released while the GL context (owned by the window) still exists.
    m_scenes.reset();
    m_spriteBatch.reset();
    m_window.reset();
    SDL_Quit();
}

int Application::Run(std::unique_ptr<Scene> firstScene) {
    m_scenes->SwitchTo(std::move(firstScene));
    m_scenes->ApplyPending();

    const double step = 1.0 / m_config.updatesPerSecond;
    // After a long stall (window dragged, breakpoint hit) do not try to catch up with
    // hundreds of updates at once; that would freeze the game even longer.
    const double maxFrameTime = 0.25;
    const double counterFrequency = static_cast<double>(SDL_GetPerformanceFrequency());

    Uint64 previousCounter = SDL_GetPerformanceCounter();
    double accumulator = 0.0;
    m_running = !m_scenes->IsEmpty();

    while (m_running) {
        const Uint64 now = SDL_GetPerformanceCounter();
        double frameTime = static_cast<double>(now - previousCounter) / counterFrequency;
        previousCounter = now;
        if (frameTime > maxFrameTime) {
            frameTime = maxFrameTime;
        }
        accumulator += frameTime;

        PollEvents();

        while (m_running && accumulator >= step) {
            m_scenes->Update(static_cast<float>(step));
            m_input.EndUpdate();
            m_scenes->ApplyPending();
            accumulator -= step;

            if (m_scenes->IsEmpty()) {
                m_running = false;
            }
        }

        if (!m_running) {
            break;
        }

        m_window->BeginFrame();
        m_scenes->Draw(static_cast<float>(accumulator / step));
        m_window->Present();
    }

    m_scenes->Clear();
    return 0;
}

void Application::PollEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            m_running = false;
            break;
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                m_window->HandleResize();
            }
            break;
        default:
            break;
        }
        m_input.HandleEvent(event);
    }
    m_input.CaptureState();
}

} // namespace engine
