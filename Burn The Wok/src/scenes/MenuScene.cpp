#include "scenes/MenuScene.h"

#include "GameConfig.h"
#include "art/PixelText.h"
#include "scenes/GameplayScene.h"

#include <engine/core/Application.h>
#include <engine/core/Log.h>
#include <engine/graphics/Camera2D.h>
#include <engine/graphics/Image.h>

#include <algorithm>
#include <cmath>
#include <exception>
#include <memory>
#include <string>
#include <string_view>

namespace game {

using engine::Color;
using engine::Key;

namespace {

float MenuScale(engine::Vec2 size) {
    return std::max(0.01f, std::min(size.x / 1280.0f, size.y / 720.0f));
}

engine::Rect StartButton(engine::Vec2 size) {
    const float s = MenuScale(size);
    return {size.x * 0.5f + 90.0f * s, size.y * 0.5f + 142.0f * s,
            350.0f * s, 72.0f * s};
}

void DrawForestTree(engine::SpriteBatch& batch, float x, float ground, float height,
                    float s, Color color) {
    const float width = height * 0.65f;
    batch.DrawRect({x - width * 0.08f, ground - height, width * 0.16f, height}, color);
    batch.DrawRect({x - width * 0.50f, ground - height * 0.88f, width, height * 0.34f}, color);
    batch.DrawRect({x - width * 0.38f, ground - height * 1.02f, width * 0.76f, height * 0.28f}, color);
    batch.DrawRect({x - width * 0.22f, ground - height * 1.10f, width * 0.44f, height * 0.20f}, color);
    batch.DrawRect({x - width * 0.58f, ground - height * 0.69f, width * 1.16f, 18.0f * s}, color);
}

} // namespace

void MenuScene::OnEnter() {
    const engine::Image logo = engine::Image::LoadFromFile(
        "assets/ui/BurnTheWo Fiery Jungle Emblem.png");
    m_logoSource = logo.GetContentBounds(16);
    m_logo = std::make_unique<engine::Texture>(logo);
    m_time = 0.0f;

    engine::Image glow(128, 128);
    for (int y = 0; y < 128; ++y) {
        for (int x = 0; x < 128; ++x) {
            const float dx = (x - 63.5f) / 63.5f;
            const float dy = (y - 63.5f) / 63.5f;
            const float strength = std::max(0.0f, 1.0f - dx * dx - dy * dy);
            glow.SetPixel(x, y, {255, 255, 255,
                static_cast<std::uint8_t>(strength * strength * 100.0f)});
        }
    }
    m_glow = std::make_unique<engine::Texture>(glow);

    App().GetWindow().SetTitle(std::string(config::WindowTitle) +
                               " - Menu | Enter/Spasi atau klik MULAI, Esc: keluar");
    App().GetWindow().SetClearColor(Color::FromHex(0x071411));

    // Menu music loops until the scene is left; it fades in so it follows the intro's fade-out smoothly.
    try {
        m_music = App().GetAudio().LoadSound("assets/audio/music/menu.wav");
        m_voice = App().GetAudio().Play(m_music, 0.0f, true);
    } catch (const std::exception& e) {
        engine::LogError("Menu music unavailable: %s", e.what()); // the menu still works in silence
    }
}

void MenuScene::OnExit() {
    App().GetAudio().Stop(m_voice);
}

void MenuScene::Update(float dt) {
    m_time += dt;
    App().GetAudio().SetVoiceVolume(m_voice, 0.7f * std::clamp(m_time / 1.5f, 0.0f, 1.0f));
    const engine::Input& input = App().GetInput();
    const bool clickStart = input.IsMousePressed(engine::MouseButton::Left) &&
                            StartButton(App().GetWindow().GetSize()).Contains(input.GetMousePosition());
    if (input.IsKeyPressed(Key::Enter) || input.IsKeyPressed(Key::NumpadEnter) ||
        input.IsKeyPressed(Key::Space) || clickStart) {
        App().GetScenes().SwitchTo(std::make_unique<GameplayScene>());
    } else if (input.IsKeyPressed(Key::Escape)) {
        App().Quit();
    }
}

void MenuScene::Draw(float /*alpha*/) {
    const engine::Vec2 size = App().GetWindow().GetSize();
    const engine::Vec2 c = size * 0.5f;
    const float s = MenuScale(size);
    const float ground = c.y + 220.0f * s;

    // Screen-space camera: 1 world unit = 1 screen pixel, origin at the top-left.
    engine::Camera2D camera;
    camera.SetViewportSize(size);
    camera.SetPosition(c);

    engine::SpriteBatch& batch = App().GetSpriteBatch();
    batch.Begin(camera.GetViewProjection());

    // A forest backdrop with a warm clearing keeps the fiery emblem in focus.
    for (int i = 0; i < 48; ++i) {
        const float t = static_cast<float>(i) / 47.0f;
        batch.DrawRect({0.0f, size.y * i / 48.0f, size.x, size.y / 48.0f + 1.0f},
            {static_cast<std::uint8_t>(7 + 7 * t), static_cast<std::uint8_t>(20 + 16 * t),
             static_cast<std::uint8_t>(17 + 9 * t), 255});
    }
    batch.Draw(*m_glow, {c.x - 560.0f * s, c.y - 370.0f * s, 1120.0f * s, 700.0f * s},
               Color::FromHex(0x738650));

    for (int layer = 0; layer < 2; ++layer) {
        const float spacing = (layer == 0 ? 105.0f : 155.0f) * s;
        const Color color = Color::FromHex(layer == 0 ? 0x102F27 : 0x091D18);
        for (int i = 0; i * spacing < size.x + spacing; ++i) {
            const float x = i * spacing + ((layer == 0) ? 0.0f : 37.0f * s);
            const float height = (180.0f + static_cast<float>((i * 71 + layer * 43) % 120)) * s;
            DrawForestTree(batch, x, ground + layer * 34.0f * s, height, s, color);
        }
    }
    batch.DrawRect({0.0f, ground + 34.0f * s, size.x,
                    std::max(0.0f, size.y - ground - 34.0f * s)}, Color::FromHex(0x091D18));

    for (int i = 0; i < 16; ++i) {
        const float progress = std::fmod(m_time * (0.045f + (i % 5) * 0.008f) + i * 0.137f, 1.0f);
        const float x = c.x + (static_cast<float>((i * 97) % 1100) - 550.0f) * s +
                        std::sin(m_time * 0.7f + i) * 14.0f * s;
        const float y = c.y + (240.0f - progress * 560.0f) * s;
        const float dot = (i % 3 == 0 ? 3.0f : 2.0f) * s;
        batch.DrawRect({x, y, dot, dot}, Color::FromHex(0xFFB95A).WithAlpha(
            static_cast<std::uint8_t>(std::sin(progress * 3.14159265f) * 150.0f)));
    }

    const Color cream = Color::FromHex(0xFFF0CB);
    const Color paper = Color::FromHex(0xE8D9AF);
    const Color shade = Color::FromHex(0xC7B58A);
    const Color ink = Color::FromHex(0x3B4031);
    const Color moss = Color::FromHex(0x738650);
    auto rect = [&](float x, float y, float width, float height, Color color) {
        batch.DrawRect({c.x + x*s, c.y + y*s, width*s, height*s}, color);
    };
    auto label = [&](std::string_view value, float x, float y, float scale, Color color) {
        art::DrawPixelText(batch, value, c.x+x*s, c.y+y*s, scale*s, color);
    };

    // Quiet editorial header, using the ranger badge from the in-game HUD.
    rect(-578, -310, 34, 40, moss);
    rect(-574, -270, 26, 4, moss);
    rect(-563, -295, 4, 20, cream);
    rect(-567, -302, 12, 5, cream);
    rect(-572, -297, 22, 6, cream);
    rect(-575, -291, 28, 5, cream);
    label("KAWASAN KONSERVASI", -528, -303, 2, shade);
    label("HUTAN TROPIS", -528, -282, 2, cream);
    label("TIM DAPUR TEDDUY", 398, -295, 2, shade);
    rect(-578, -245, 1156, 1, Color::FromHex(0xC7B58A, 55));
    const float logoWidth = 920.0f * s;
    const float logoHeight = std::min(345.0f * s, logoWidth * m_logoSource.h / m_logoSource.w);
    const float actualWidth = logoHeight * m_logoSource.w / m_logoSource.h;
    const float bob = 0.0f;
    const engine::Rect logoRect{c.x - actualWidth * 0.5f,
                                c.y - 85.0f * s - logoHeight * 0.5f + bob,
                                actualWidth, logoHeight};
    batch.Draw(*m_logo, {logoRect.x + 6.0f * s, logoRect.y + 10.0f * s,
                         actualWidth, logoHeight}, m_logoSource, Color{0, 0, 0, 150});
    batch.Draw(*m_logo, logoRect, m_logoSource);
    // A field note balances the start action without enclosing the logo in a card.
    rect(-436, 122, 344, 136, Color::FromHex(0x071411, 150));
    rect(-442, 116, 344, 136, paper);
    rect(-442, 116, 5, 136, shade);
    rect(-419, 110, 64, 13, Color::FromHex(0xB9B58A));
    label("SEBELUM MASUK HUTAN", -422, 137, 2, ink);
    rect(-422, 160, 302, 1, shade);
    auto key = [&](std::string_view value, float x, float y, float width) {
        rect(x, y+3, width, 21, shade);
        rect(x, y, width, 21, cream);
        label(value, x+6, y+4, 2, ink);
    };
    key("WASD", -422, 175, 58);
    label("GERAK", -346, 180, 2, ink);
    label("ATAU TOMBOL PANAH", -346, 201, 1, ink);
    key("SCROLL", -422, 220, 82);
    label("ZOOM TAMPILAN", -324, 225, 2, ink);

    const engine::Rect button = StartButton(size);
    const bool hovered = button.Contains(App().GetInput().GetMousePosition());
    label("HUTAN MENUNGGUMU", 90, 116, 2, shade);
    batch.DrawRect({button.x, button.y+5*s, button.w, button.h}, Color::FromHex(0x071411));
    batch.DrawRect(button, hovered ? cream : paper);
    batch.DrawRect({button.x, button.y, 6*s, button.h}, moss);
    label("MULAI JELAJAH", 113, 168, 3, ink);
    // Pixel trail arrow on the far end of the paper tab.
    rect(400, 175, 22, 4, ink);
    for (int i=0; i<4; ++i)
        rect(416+i*3.0f, 166+i*3.0f, 3, 22-i*6.0f, ink);
    label("ENTER - SPASI", 90, 232, 2, hovered ? cream : shade);

    rect(-578, 287, 1156, 1, Color::FromHex(0xC7B58A, 55));
    label("JAGA HUTAN - PADAMKAN API", -578, 310, 2, shade);
    rect(422, 303, 46, 25, Color::FromHex(0x3B4031));
    label("ESC", 428, 309, 2, cream);
    label("KELUAR", 484, 309, 2, shade);
    batch.End();
}

} // namespace game
