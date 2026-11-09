#include "scenes/ResultScene.h"

#include "GameConfig.h"
#include "scenes/MenuScene.h"

#include <engine/core/Application.h>
#include <engine/graphics/Camera2D.h>

#include <memory>
#include <string>

namespace game {

using engine::Color;
using engine::Key;

void ResultScene::OnEnter() {
    const bool win = m_outcome == Outcome::Win;
    App().GetWindow().SetTitle(std::string(config::WindowTitle) + (win ? " - MENANG" : " - KALAH") +
                               " | Enter: kembali ke menu");
    App().GetWindow().SetClearColor(win ? Color::FromHex(0x265C42) : Color::FromHex(0x3E2731));
}

void ResultScene::Update(float /*dt*/) {
    const engine::Input& input = App().GetInput();
    if (input.IsKeyPressed(Key::Enter) || input.IsKeyPressed(Key::NumpadEnter) || input.IsKeyPressed(Key::Space) ||
        input.IsKeyPressed(Key::Escape)) {
        App().GetScenes().SwitchTo(std::make_unique<MenuScene>());
    }
}

void ResultScene::Draw(float /*alpha*/) {
    const engine::Vec2 size = App().GetWindow().GetSize();
    const engine::Vec2 c = size * 0.5f;

    engine::Camera2D camera;
    camera.SetViewportSize(size);
    camera.SetPosition(c);

    engine::SpriteBatch& batch = App().GetSpriteBatch();
    batch.Begin(camera.GetViewProjection());

    if (m_outcome == Outcome::Win) {
        // Yellow plus-shaped "star".
        const Color star = Color::FromHex(0xFEE761);
        batch.DrawRect({c.x - 20.0f, c.y - 80.0f, 40.0f, 160.0f}, star);
        batch.DrawRect({c.x - 80.0f, c.y - 20.0f, 160.0f, 40.0f}, star);
    } else {
        // Orange "X" built from small squares along both diagonals.
        const Color cross = Color::FromHex(0xF77622);
        for (int i = -6; i <= 6; ++i) {
            const float d = i * 12.0f;
            batch.DrawRect({c.x + d - 10.0f, c.y + d - 10.0f, 20.0f, 20.0f}, cross);
            batch.DrawRect({c.x + d - 10.0f, c.y - d - 10.0f, 20.0f, 20.0f}, cross);
        }
    }

    batch.End();
}

} // namespace game
