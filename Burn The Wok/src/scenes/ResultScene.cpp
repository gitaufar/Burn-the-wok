#include "scenes/ResultScene.h"

#include "GameConfig.h"
#include "art/PixelText.h"
#include "scenes/MenuScene.h"

#include <engine/core/Application.h>
#include <engine/graphics/Camera2D.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <utility>

namespace game {

using engine::Color;
using engine::Key;

void ResultScene::OnEnter() {
    const bool win = m_outcome == Outcome::Win;
    App().GetWindow().SetTitle(std::string(config::WindowTitle) + (win ? " - MENANG" : " - KALAH") +
                               " | Enter: kembali ke menu");
    App().GetWindow().SetClearColor(win ? Color::FromHex(0x265C42) : Color::FromHex(0x3E2731));

    m_sfx = std::make_unique<SoundBank>(App().GetAudio());
    m_sfx->Load({"win", "lose", "ui_click"});
    m_sfx->Play(win ? "win" : "lose", config::sound::Result);
}

void ResultScene::Update(float /*dt*/) {
    const engine::Input& input = App().GetInput();
    if (input.IsKeyPressed(Key::Enter) || input.IsKeyPressed(Key::NumpadEnter) || input.IsKeyPressed(Key::Space) ||
        input.IsKeyPressed(Key::Escape)) {
        m_sfx->Play("ui_click", config::sound::UiClick);
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

    // Pixel scale that fits the window (same idea as the gameplay HUD).
    const float s = std::max(1.0f, std::floor(std::min(size.x / 640.0f, size.y / 360.0f)));
    const float symbolY = c.y - 70.0f * s;
    if (m_outcome == Outcome::Win) {
        // Yellow plus-shaped "star".
        const Color star = Color::FromHex(0xFEE761);
        batch.DrawRect({c.x - 8.0f * s, symbolY - 32.0f * s, 16.0f * s, 64.0f * s}, star);
        batch.DrawRect({c.x - 32.0f * s, symbolY - 8.0f * s, 64.0f * s, 16.0f * s}, star);
    } else {
        // Orange "X" built from small squares along both diagonals.
        const Color cross = Color::FromHex(0xF77622);
        for (int i = -6; i <= 6; ++i) {
            const float d = i * 5.0f * s;
            batch.DrawRect({c.x + d - 4.0f * s, symbolY + d - 4.0f * s, 8.0f * s, 8.0f * s}, cross);
            batch.DrawRect({c.x + d - 4.0f * s, symbolY - d - 4.0f * s, 8.0f * s, 8.0f * s}, cross);
        }
    }

    // Centered pixel-font line; glyphs are 6 units wide including spacing.
    auto line = [&](const std::string& text, float y, float scale, Color color) {
        const float width = static_cast<float>(text.size()) * 6.0f * scale - scale;
        art::DrawPixelText(batch, text, std::round(c.x - width * 0.5f), std::round(y), scale, color);
    };
    const Color cream = Color::FromHex(0xFFF0CB);
    const Color muted = Color::FromHex(0xD8C9A3);
    float y = c.y - 20.0f * s;
    line(m_outcome == Outcome::Win ? "MENANG" : "KALAH", y, 4.0f * s, cream);
    y += 40.0f * s;
    if (!m_summary.reason.empty()) {
        line(m_summary.reason, y, 2.0f * s, m_outcome == Outcome::Win ? Color::FromHex(0xB6D98A)
                                                                       : Color::FromHex(0xF5A26B));
        y += 26.0f * s;
    }
    if (m_summary.hasStats) {
        line("HUTAN TERSISA: " + std::to_string(m_summary.forestPercent) + "%", y, s, muted);
        line("LAHAN SAWIT: " + std::to_string(m_summary.sawitPercent) + "%", y + 12.0f * s, s, muted);
        line("API DIPADAMKAN: " + std::to_string(m_summary.firesPutOut), y + 24.0f * s, s, muted);
        line("SISA WAKTU: " + std::to_string(m_summary.secondsLeft) + " DETIK", y + 36.0f * s, s, muted);
        y += 54.0f * s;
    }
    line("ENTER: KEMBALI KE MENU", y, s, Color::FromHex(0xFFF0CB, 150));

    batch.End();
}

} // namespace game
