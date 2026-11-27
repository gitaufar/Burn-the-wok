#include "scenes/LoadingScene.h"

#include "art/FieldIcons.h"
#include "art/ForestBackdrop.h"
#include "art/PixelText.h"
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

constexpr float StageFillSeconds = 1.6f; // long enough to read the briefing
constexpr float QuickFillSeconds = 0.6f; // other screens: just long enough to be seen
constexpr float HoldSeconds = 0.15f;     // the full trail stays on screen before moving on
constexpr float FadeSeconds = 0.25f;

// One tip per stage, about what is new there (two short lines each).
constexpr const char* Tips[][2] = {
    {"TAHAN KLIK KIRI UNTUK MENYEMPROT.", "ISI TANGKI LAGI DI TEPI KOLAM."},
    {"ADA YANG SENGAJA MENYALAKAN API", "DAN MENANAM BIBIT SAWIT."},
    {"WOWO DAN TEDDUY AKAN MENGHADANG.", "CARI JALAN MEMUTAR."},
};

const Color Cream = Color::FromHex(0xFFF0CB);
const Color Paper = Color::FromHex(0xE8D9AF);
const Color Shade = Color::FromHex(0xC7B58A);
const Color Ink = Color::FromHex(0x3B4031);
const Color Moss = Color::FromHex(0x738650);
const Color Ember = Color::FromHex(0x8D382A);

// Text centered on x (the pixel font advances 6 units per character).
void DrawCentered(engine::SpriteBatch& batch, std::string_view value, float x, float y,
                  float scale, Color color) {
    const float width = (value.size() * 6.0f - 1.0f) * scale;
    art::DrawPixelText(batch, value, x - width * 0.5f, y, scale, color);
}

} // namespace

std::unique_ptr<engine::Scene> LoadStage(config::StageId stage) {
    return std::make_unique<LoadingScene>(
        [stage] { return std::make_unique<GameplayScene>(stage); }, "MENYIAPKAN PATROLI", stage);
}

float LoadingScene::FillSeconds() const {
    return m_stage ? StageFillSeconds : QuickFillSeconds;
}

void LoadingScene::OnEnter() {
    for (int i = 0; i < 4; ++i) {
        const auto image = engine::Image::LoadFromFile(
            "assets/characters/ranger/right_" + std::to_string(i + 1) + ".png");
        if (i == 0) m_rangerBounds = image.GetContentBounds(128);
        m_ranger[i] = std::make_unique<engine::Texture>(image);
    }
    const auto fire = engine::Image::LoadFromFile("assets/items/fire.png");
    m_fireSource = fire.GetContentBounds(16);
    m_fire = std::make_unique<engine::Texture>(fire);

    App().GetWindow().SetTitle(std::string(config::WindowTitle) + " - Memuat");
    App().GetWindow().SetClearColor(Color::FromHex(0x071411));
}

void LoadingScene::Update(float dt) {
    m_time += dt;
    const engine::Input& input = App().GetInput();
    const bool skip = input.IsKeyPressed(Key::Enter) || input.IsKeyPressed(Key::NumpadEnter) ||
                      input.IsKeyPressed(Key::Space) || input.IsMousePressed(engine::MouseButton::Left);
    if (skip) m_time = std::max(m_time, FillSeconds()); // jump to the full trail, then move on
    if (m_time >= FillSeconds() + HoldSeconds) {
        App().GetScenes().SwitchTo(m_next());
    }
}

void LoadingScene::DrawBriefing(engine::SpriteBatch& batch, engine::Vec2 c, float s) const {
    const auto& stage = config::GetStage(*m_stage);
    const int stageIndex = static_cast<int>(*m_stage);
    auto rect = [&](float x, float y, float width, float height, Color color) {
        batch.DrawRect({c.x + x * s, c.y + y * s, width * s, height * s}, color);
    };
    auto centered = [&](std::string_view value, float x, float y, float scale, Color color) {
        DrawCentered(batch, value, c.x + x * s, c.y + y * s, scale * s, color);
    };

    // Patrol briefing sheet, in the style of the menu's field note.
    rect(-324, -222, 660, 268, Color::FromHex(0x071411, 150));
    rect(-330, -230, 660, 268, Paper);
    rect(-330, -230, 6, 268, Moss);
    rect(-52, -238, 104, 16, Color::FromHex(0xB9B58A)); // tape
    centered("SURAT TUGAS / STAGE " + std::to_string(stageIndex + 1), 0, -200, 2, Ink);
    centered(stage.name, 0, -170, 5, Ember);
    rect(-280, -116, 560, 2, Shade);

    const int seconds = static_cast<int>(stage.duration);
    const std::string clock = std::to_string(seconds / 60) + ":" +
                              (seconds % 60 < 10 ? "0" : "") + std::to_string(seconds % 60);
    const std::string facts[] = {"WAKTU " + clock, std::to_string(stage.initialFires) + " TITIK API",
                                 "SATWA MIN. " + std::to_string(stage.rescueTarget)};
    for (int i = 0; i < 3; ++i) centered(facts[i], -200.0f + i * 200.0f, -96, 2, Ink);

    rect(-280, -62, 560, 2, Shade);
    art::DrawFieldIcon(batch, art::FieldIcon::Leaf, c.x - 280 * s, c.y - 40 * s, 2.5f * s, Moss);
    centered(Tips[stageIndex][0], 12, -42, 2, Ink);
    centered(Tips[stageIndex][1], 12, -18, 2, Ink);
}

void LoadingScene::Draw(float /*alpha*/) {
    const engine::Vec2 size = App().GetWindow().GetSize();
    const engine::Vec2 c = size * 0.5f;
    const float s = std::max(0.01f, std::min(size.x / 1280.0f, size.y / 720.0f));
    const float progress = std::clamp(m_time / FillSeconds(), 0.0f, 1.0f);

    engine::Camera2D camera;
    camera.SetViewportSize(size);
    camera.SetPosition(c);

    engine::SpriteBatch& batch = App().GetSpriteBatch();
    batch.Begin(camera.GetViewProjection());
    art::DrawForestSky(batch, size);
    art::DrawForestSilhouettes(batch, size, c.y + 220.0f * s, s);

    // Embers drifting up, as on the main menu.
    for (int i = 0; i < 16; ++i) {
        const float rise = std::fmod(m_time * (0.12f + (i % 5) * 0.02f) + i * 0.137f, 1.0f);
        const float x = c.x + (static_cast<float>((i * 97) % 1100) - 550.0f) * s;
        const float y = c.y + (300.0f - rise * 560.0f) * s;
        const float dot = (i % 3 == 0 ? 3.0f : 2.0f) * s;
        batch.DrawRect({x, y, dot, dot}, Color::FromHex(0xFFB95A).WithAlpha(
            static_cast<std::uint8_t>(std::sin(rise * 3.14159265f) * 150.0f)));
    }

    auto rect = [&](float x, float y, float width, float height, Color color) {
        batch.DrawRect({c.x + x * s, c.y + y * s, width * s, height * s}, color);
    };
    if (m_stage) DrawBriefing(batch, c, s);

    // Progress trail: the ranger walks toward the fire.
    constexpr float TrailLeft = -300.0f;
    constexpr float TrailWidth = 600.0f;
    // Below the briefing sheet, or in the middle of the screen without one.
    const float TrailY = m_stage ? 156.0f : 40.0f;
    rect(TrailLeft - 4, TrailY - 4, TrailWidth + 8, 20, Color::FromHex(0x071411));
    rect(TrailLeft, TrailY, TrailWidth, 12, Ink);
    rect(TrailLeft, TrailY, TrailWidth * progress, 12, Moss);
    rect(TrailLeft, TrailY, TrailWidth * progress, 3, Color::FromHex(0xB9B58A));

    const float flicker = 1.0f + 0.06f * std::sin(m_time * 14.0f);
    const float fireHeight = 56.0f * flicker;
    const float fireWidth = fireHeight * m_fireSource.w / m_fireSource.h;
    batch.Draw(*m_fire, {c.x + (TrailLeft + TrailWidth + 14) * s, c.y + (TrailY + 12 - fireHeight) * s,
                         fireWidth * s, fireHeight * s}, m_fireSource);

    // All walk frames share one canvas, so one scale and anchor keep the ranger steady.
    const int frame = progress < 1.0f ? static_cast<int>(m_time / 0.12f) % 4 : 0;
    const float rangerScale = 84.0f * s / m_rangerBounds.h;
    const float rangerX = c.x + (TrailLeft + TrailWidth * progress) * s;
    const float feetY = c.y + (TrailY + 2) * s;
    const engine::Texture& ranger = *m_ranger[frame];
    batch.Draw(ranger, {rangerX - (m_rangerBounds.x + m_rangerBounds.w * 0.5f) * rangerScale,
                        feetY - (m_rangerBounds.y + m_rangerBounds.h) * rangerScale,
                        ranger.GetWidth() * rangerScale, ranger.GetHeight() * rangerScale});

    // Anchored by the three-dot width so the animated dots do not shift the text.
    const std::string message = m_message;
    const float messageWidth = ((message.size() + 3) * 6.0f - 1.0f) * 2.0f;
    art::DrawPixelText(batch, message + std::string(1 + static_cast<int>(m_time / 0.2f) % 3, '.'),
                       c.x - messageWidth * 0.5f * s, c.y + (TrailY + 40) * s, 2 * s, Cream);
    // Below one screen pixel per font pixel the small hint would break up, so it stops shrinking.
    if (m_stage) {
        DrawCentered(batch, "SPASI / KLIK UNTUK LEWATI", c.x, c.y + 300 * s, std::max(1.0f, 1.5f * s), Shade);
    }

    // Fade in from the dark forest.
    const float cover = 1.0f - std::clamp(m_time / FadeSeconds, 0.0f, 1.0f);
    if (cover > 0.0f) {
        batch.DrawRect({0.0f, 0.0f, size.x, size.y},
                       Color::FromHex(0x071411, static_cast<std::uint8_t>(cover * 255.0f)));
    }
    batch.End();
}

} // namespace game
