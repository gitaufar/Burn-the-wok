#include "scenes/MenuScene.h"

#include "GameConfig.h"
#include "scenes/GameplayScene.h"

#include <engine/core/Application.h>
#include <engine/graphics/Camera2D.h>

#include <cmath>
#include <memory>
#include <string>

namespace game {

using engine::Color;
using engine::Key;

void MenuScene::OnEnter() {
    App().GetWindow().SetTitle(std::string(config::WindowTitle) +
                               " - Menu | Enter: mulai, Esc: keluar");
    App().GetWindow().SetClearColor(Color::FromHex(0x181425));
}

void MenuScene::Update(float dt) {
    m_time += dt;

    const engine::Input& input = App().GetInput();
    if (input.IsKeyPressed(Key::Enter) || input.IsKeyPressed(Key::Space)) {
        App().GetScenes().SwitchTo(std::make_unique<GameplayScene>());
    } else if (input.IsKeyPressed(Key::Escape)) {
        App().Quit();
    }
}

void MenuScene::Draw(float /*alpha*/) {
    const engine::Vec2 size = App().GetWindow().GetSize();
    const engine::Vec2 c = size * 0.5f;
    const float ground = c.y + 60.0f;

    // Screen-space camera: 1 world unit = 1 screen pixel, origin at the top-left.
    engine::Camera2D camera;
    camera.SetViewportSize(size);
    camera.SetPosition(c);

    engine::SpriteBatch& batch = App().GetSpriteBatch();
    batch.Begin(camera.GetViewProjection());

    batch.DrawRect({0.0f, ground, size.x, size.y - ground}, Color::FromHex(0x193C3E));

    // Tree-line silhouette.
    for (int i = 0; i * 48.0f < size.x; ++i) {
        const float h = 40.0f + static_cast<float>((i * 37) % 50);
        batch.DrawRect({i * 48.0f + 8.0f, ground - h, 32.0f, h}, Color::FromHex(0x265C42));
    }

    // Flickering "logo" flame: three layers that all stand on the ground line.
    const float flicker = std::sin(m_time * 9.0f) * 6.0f + std::sin(m_time * 23.0f) * 3.0f;
    batch.DrawRect({c.x - 60.0f, c.y - 40.0f - flicker, 120.0f, 100.0f + flicker},
                   Color::FromHex(0xE43B44));
    batch.DrawRect({c.x - 40.0f, c.y - 10.0f - flicker * 0.6f, 80.0f, 70.0f + flicker * 0.6f},
                   Color::FromHex(0xF77622));
    batch.DrawRect({c.x - 20.0f, c.y + 20.0f - flicker * 0.3f, 40.0f, 40.0f + flicker * 0.3f},
                   Color::FromHex(0xFEE761));

    // Blinking "press Enter" bar.
    if (std::fmod(m_time, 1.0f) < 0.6f) {
        batch.DrawRect({c.x - 100.0f, ground + 50.0f, 200.0f, 16.0f}, Color::White().WithAlpha(200));
    }

    batch.End();
}

} // namespace game
