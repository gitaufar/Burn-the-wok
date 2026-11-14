#include "GameConfig.h"
#include "scenes/IntroScene.h"
#include "scenes/MenuScene.h"

#include <engine/core/Application.h>
#include <engine/core/Log.h>

#include <exception>
#include <cstring>
#include <memory>

int main(int argc, char* argv[]) {
    // "--skip-intro" goes straight to the menu (handy while developing).
    bool skipIntro = false;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--skip-intro") == 0) skipIntro = true;
    }

    try {
        engine::AppConfig config;
        config.title = game::config::WindowTitle;
        config.width = game::config::WindowWidth;
        config.height = game::config::WindowHeight;

        engine::Application app(config);
        if (skipIntro) {
            return app.Run(std::make_unique<game::MenuScene>());
        }
        return app.Run(std::make_unique<game::IntroScene>());
    } catch (const std::exception& e) {
        engine::ReportFatalError(e.what());
        return 1;
    }
}
