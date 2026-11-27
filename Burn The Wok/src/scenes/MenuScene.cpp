#include "scenes/MenuScene.h"
#include "scenes/StoreScene.h"
#include "progress/SaveData.h"
#include "art/FieldIcons.h"
#include "art/ForestBackdrop.h"

#include "GameConfig.h"
#include "art/PixelText.h"
#include "scenes/LoadingScene.h"

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

engine::Rect MenuButton(engine::Vec2 size, int index) {
    const auto c = size * 0.5f;
    const float s = MenuScale(size);
    return {c.x+90*s, c.y+(126+index*37)*s, 350*s, 32*s};
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
                               " - Menu | Panah dan Enter, Esc: keluar");
    App().GetWindow().SetClearColor(Color::FromHex(0x071411));

    // Menu music loops until the scene is left; it fades in so it follows the intro's fade-out smoothly.
    try {
        m_music = App().GetAudio().LoadSound("assets/audio/music/menu.wav");
        m_voice = App().GetAudio().Play(m_music, 0.0f, true);
    } catch (const std::exception& e) {
        engine::LogError("Menu music unavailable: %s", e.what()); // the menu still works in silence
    }

    m_sfx = std::make_unique<SoundBank>(App().GetAudio());
    m_sfx->Load({"ui_click", "ui_hover", "purchase_fail"});
}

void MenuScene::OnExit() {
    App().GetAudio().Stop(m_voice);
}

void MenuScene::Update(float dt) {
    m_time += dt;
    App().GetAudio().SetVoiceVolume(m_voice, config::sound::MenuMusic * std::clamp(m_time / 1.5f, 0.0f, 1.0f));
    const engine::Input& input = App().GetInput();
    const auto size = App().GetWindow().GetSize();
    const int count = m_page == Page::Reset ? 2 : m_page == Page::Stages ? 3 : 4;
    int hovered = -1;
    for (int i = 0; i < count; ++i)
        if (MenuButton(size, i).Contains(input.GetMousePosition())) hovered = i;
    const int previousFocus = m_focus;
    if (input.IsKeyPressed(Key::Up)) m_focus = (m_focus+count-1)%count;
    if (input.IsKeyPressed(Key::Down)) m_focus = (m_focus+1)%count;
    if ((hovered >= 0 && hovered != m_hovered) || previousFocus != m_focus)
        m_sfx->Play("ui_hover", config::sound::UiHover);
    m_hovered = hovered;

    if (input.IsKeyPressed(Key::Escape)) {
        if (m_page == Page::Main) App().Quit();
        else { m_page = Page::Main; m_focus = 0; m_resetFailed = false; }
    } else if (hovered >= 0 && input.IsMousePressed(engine::MouseButton::Left)) {
        Select(hovered);
    } else if (input.IsKeyPressed(Key::Enter) || input.IsKeyPressed(Key::NumpadEnter) ||
               input.IsKeyPressed(Key::Space)) {
        Select(m_focus);
    }
}

void MenuScene::Select(int index) {
    auto& save = SaveData::Current();
    if ((m_page == Page::Stages && index > save.unlocked) ||
        (m_page == Page::Main && index == 2 && !save.cleared[0])) {
        m_sfx->Play("purchase_fail", 0.5f);
        return;
    }
    m_sfx->Play("ui_click", config::sound::UiClick);
    if (m_page == Page::Reset) {
        if (index == 1 && !save.Commit(SaveData{})) {
            m_resetFailed = true;
            m_sfx->Play("purchase_fail", 0.5f);
            return;
        }
        m_page = Page::Main;
        m_focus = 0;
        m_resetFailed = false;
    } else if (m_page == Page::Stages || index == 0) {
        const int stage = m_page == Page::Stages ? index : save.unlocked;
        App().GetScenes().SwitchTo(LoadStage(static_cast<config::StageId>(stage)));
    } else if (index == 2) {
        App().GetScenes().SwitchTo(std::make_unique<LoadingScene>(
            [] { return std::make_unique<StoreScene>(); }, "MEMBUKA GUDANG"));
    } else {
        m_page = index == 1 ? Page::Stages : Page::Reset;
        m_focus = 0;
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
    art::DrawForestSky(batch, size);
    batch.Draw(*m_glow, {c.x - 560.0f * s, c.y - 370.0f * s, 1120.0f * s, 700.0f * s},
               Color::FromHex(0x738650));

    art::DrawForestSilhouettes(batch, size, ground, s);

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
    key("SPASI", -422, 220, 72);
    label("TAHAN / TOLONG SATWA", -338, 225, 1.6f, ink);

    const auto& save = SaveData::Current();
    const bool chooseStage = m_page == Page::Stages;
    const bool reset = m_page == Page::Reset;
    const int count = reset ? 2 : chooseStage ? 3 : 4;
    label(reset ? "HAPUS SELURUH PROGRES?" : chooseStage ? "PILIH WILAYAH PATROLI" :
          "ENTER PILIH / PANAH ATAS BAWAH", 90, 105, 1.5f, shade);
    const bool started = save.cleared[0] || save.points > 0;
    const std::string actions[] = {started ? "LANJUTKAN / STAGE " + std::to_string(save.unlocked+1) : "MULAI JELAJAH",
        "PILIH STAGE", "TOKO PERLENGKAPAN", "RESET PROGRES"};
    for (int i = 0; i < count; ++i) {
        const auto button = MenuButton(size, i);
        const float y = 126+i*37.0f;
        const bool locked = chooseStage ? i > save.unlocked : !reset && i == 2 && !save.cleared[0];
        const bool focused = i == m_focus || i == m_hovered;
        const Color front = locked ? Color::FromHex(0x5E5E4B) : ink;
        rect(93, y+4, 350, 32, Color::FromHex(0x071411));
        batch.DrawRect(button, focused ? cream : locked ? shade : paper);
        rect(90, y, 5, 32, focused ? moss : shade);
        const std::string title = reset ? (i == 0 ? "BATAL" : "YA / RESET PROGRES") :
            chooseStage ? std::to_string(i+1) + " / " + config::Stages[i].name : actions[i];
        label(title, 107, y+9, 2, front);
        const auto icon = [&](art::FieldIcon glyph, float x, Color color) {
            art::DrawFieldIcon(batch, glyph, c.x+x*s, c.y+(y+9)*s, 1.65f*s, color);
        };
        if (locked) {
            icon(art::FieldIcon::Lock, 412, front);
        } else if (chooseStage) {
            for (int n = 0; n < 3; ++n) icon(art::FieldIcon::Star, 378+n*16.0f,
                n < save.stars[i] ? Color::FromHex(0x996423) : shade);
        } else if (!reset && i == 2 && save.storeNew && std::fmod(m_time, 0.95f) < 0.64f) {
            label("BARU!", 373, y+10, 2, Color::FromHex(0x8D382A));
        }
    }
    if (reset) label(m_resetFailed ? "GAGAL MENYIMPAN RESET" : "SALDO, STAGE, DAN PERLENGKAPAN DIHAPUS.",
                     90, 208, 1.4f, shade);

    rect(-578, 287, 1156, 1, Color::FromHex(0xC7B58A, 55));
    art::DrawFieldIcon(batch, art::FieldIcon::Leaf, c.x-578*s, c.y+308*s, 2*s, Color::FromHex(0xE9BE70));
    label("SALDO " + std::to_string(save.points), -553, 310, 2, cream);
    if (save.campaignSeconds > 0) {
        art::DrawFieldIcon(batch, art::FieldIcon::Megaphone, c.x-332*s, c.y+308*s, 2*s, Color::FromHex(0xE9BE70));
        label("KAMPANYE AKTIF " + CampaignClock(save.campaignSeconds), -303, 310, 2, shade);
    }
    rect(422, 303, 46, 25, Color::FromHex(0x3B4031));
    label("ESC", 428, 309, 2, cream);
    label(m_page == Page::Main ? "KELUAR" : "KEMBALI", 484, 309, 2, shade);
    batch.End();
}

} // namespace game
