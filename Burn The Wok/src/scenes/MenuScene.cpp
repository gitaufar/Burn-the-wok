#include "scenes/MenuScene.h"

#include "GameConfig.h"
#include "scenes/GameplayScene.h"

#include <engine/core/Application.h>
#include <engine/graphics/Camera2D.h>
#include <engine/graphics/Image.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <string_view>

namespace game {

using engine::Color;
using engine::Key;

namespace {

// Local 5x7 labels keep the startup menu readable without a font dependency.
constexpr unsigned char Glyphs[26][7] = {
    {14,17,17,31,17,17,17}, {30,17,17,30,17,17,30},
    {14,17,16,16,16,17,14}, {30,17,17,17,17,17,30},
    {31,16,16,30,16,16,31}, {31,16,16,30,16,16,16},
    {14,17,16,23,17,17,14}, {17,17,17,31,17,17,17},
    {31,4,4,4,4,4,31}, {7,2,2,2,18,18,12},
    {17,18,20,24,20,18,17}, {16,16,16,16,16,16,31},
    {17,27,21,21,17,17,17}, {17,25,21,19,17,17,17},
    {14,17,17,17,17,17,14}, {30,17,17,30,16,16,16},
    {14,17,17,17,21,18,13}, {30,17,17,30,20,18,17},
    {15,16,16,14,1,1,30}, {31,4,4,4,4,4,4},
    {17,17,17,17,17,17,14}, {17,17,17,17,17,10,4},
    {17,17,17,21,21,21,10}, {17,17,10,4,10,17,17},
    {17,17,10,4,4,4,4}, {31,1,2,4,8,16,31}
};

void DrawLabel(engine::SpriteBatch& batch, std::string_view text, float centerX,
               float y, float scale, Color color) {
    float x = centerX - (static_cast<float>(text.size()) * 6.0f - 1.0f) * scale * 0.5f;
    for (char ch : text) {
        if (ch >= 'A' && ch <= 'Z') {
            for (int row = 0; row < 7; ++row) {
                for (int col = 0; col < 5; ++col) {
                    if (Glyphs[ch - 'A'][row] & (1 << (4 - col))) {
                        batch.DrawRect({x + col * scale, y + row * scale, scale, scale}, color);
                    }
                }
            }
        }
        x += 6.0f * scale;
    }
}

float MenuScale(engine::Vec2 size) {
    return std::max(0.01f, std::min(size.x / 1280.0f, size.y / 720.0f));
}

engine::Rect StartButton(engine::Vec2 size) {
    const float s = MenuScale(size);
    return {size.x * 0.5f - 164.0f * s, size.y * 0.5f + 150.0f * s,
            328.0f * s, 64.0f * s};
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
}

void MenuScene::Update(float dt) {
    m_time += dt;
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
               Color::FromHex(0xBC6833));

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

    for (int i = 0; i < 38; ++i) {
        const float progress = std::fmod(m_time * (0.045f + (i % 5) * 0.008f) + i * 0.137f, 1.0f);
        const float x = c.x + (static_cast<float>((i * 97) % 1100) - 550.0f) * s +
                        std::sin(m_time * 0.7f + i) * 14.0f * s;
        const float y = c.y + (240.0f - progress * 560.0f) * s;
        const float dot = (i % 3 == 0 ? 3.0f : 2.0f) * s;
        batch.DrawRect({x, y, dot, dot}, Color::FromHex(0xFFB95A).WithAlpha(
            static_cast<std::uint8_t>(std::sin(progress * 3.14159265f) * 150.0f)));
    }

    DrawLabel(batch, "SELAMATKAN HUTAN", c.x, c.y - 292.0f * s, 2.0f * s,
              Color::FromHex(0xA6B99A));
    batch.DrawRect({c.x - 165.0f * s, c.y - 267.0f * s, 330.0f * s, s},
                    Color::FromHex(0xA27840).WithAlpha(100));

    const float logoWidth = 920.0f * s;
    const float logoHeight = std::min(345.0f * s, logoWidth * m_logoSource.h / m_logoSource.w);
    const float actualWidth = logoHeight * m_logoSource.w / m_logoSource.h;
    const float bob = std::sin(m_time * 1.4f) * 2.0f * s;
    const engine::Rect logoRect{c.x - actualWidth * 0.5f,
                                c.y - 85.0f * s - logoHeight * 0.5f + bob,
                                actualWidth, logoHeight};
    batch.Draw(*m_logo, {logoRect.x + 6.0f * s, logoRect.y + 10.0f * s,
                         actualWidth, logoHeight}, m_logoSource, Color{0, 0, 0, 150});
    batch.Draw(*m_logo, logoRect, m_logoSource);
    DrawLabel(batch, "JAGA HUTAN   PADAMKAN API", c.x, c.y + 106.0f * s,
              2.0f * s, Color::FromHex(0xD1C9AC));

    const engine::Rect button = StartButton(size);
    const bool hovered = button.Contains(App().GetInput().GetMousePosition());
    batch.Draw(*m_glow, {c.x - 250.0f * s, button.y - 56.0f * s, 500.0f * s, 180.0f * s},
               Color::FromHex(hovered ? 0xECA237 : 0x965524));
    batch.DrawRect({button.x, button.y + 6.0f * s, button.w, button.h}, Color::FromHex(0x562B16));
    batch.DrawRect(button, Color::FromHex(0xC58636));
    batch.DrawRect({button.x + 2.0f * s, button.y + 2.0f * s,
                    button.w - 4.0f * s, button.h - 5.0f * s},
                   Color::FromHex(hovered ? 0xFFD16A : 0xF2AC48));
    batch.DrawRect({button.x + 4.0f * s, button.y + 3.0f * s,
                    button.w - 8.0f * s, 2.0f * s}, Color::FromHex(0xFFE3A0));
    DrawLabel(batch, "MULAI", c.x + 10.0f * s, button.y + 20.0f * s, 3.5f * s,
              Color::FromHex(0x302516));
    for (int i = 0; i < 6; ++i) {
        batch.DrawRect({c.x - 78.0f * s + i * 2.0f * s, button.y + (21.0f + i * 2.0f) * s,
                        2.0f * s, (22.0f - i * 4.0f) * s}, Color::FromHex(0x302516));
    }

    DrawLabel(batch, "ENTER ATAU SPASI ATAU KLIK", c.x, button.Bottom() + 20.0f * s,
              1.5f * s, Color::FromHex(0xD1C9AC));
    DrawLabel(batch, "WASD ATAU PANAH UNTUK GERAK", c.x - 154.0f * s, c.y + 301.0f * s,
              1.25f * s, Color::FromHex(0x829B88));
    DrawLabel(batch, "ESC UNTUK KELUAR", c.x + 192.0f * s, c.y + 301.0f * s,
              1.25f * s, Color::FromHex(0x829B88));

    batch.End();
}

} // namespace game
