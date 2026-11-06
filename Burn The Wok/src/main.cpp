#include "GameConfig.h"
#include "scenes/MenuScene.h"

#include <engine/core/Application.h>
#include <engine/core/Log.h>

#include <exception>
#include <memory>

int main(int /*argc*/, char* /*argv*/[]) {
    try {
        engine::AppConfig config;
        config.title = game::config::WindowTitle;
        config.width = game::config::WindowWidth;
        config.height = game::config::WindowHeight;

        engine::Application app(config);
        return app.Run(std::make_unique<game::MenuScene>());
    } catch (const std::exception& e) {
        engine::ReportFatalError(e.what());
        return 1;
    }
}
