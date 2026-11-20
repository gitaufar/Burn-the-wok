#include "scenes/ResultScene.h"

#include "GameConfig.h"
#include "systems/StarRating.h"
#include "art/PixelText.h"
#include "scenes/MenuScene.h"
#include "scenes/GameplayScene.h"
#include <engine/core/Application.h>
#include <engine/graphics/Camera2D.h>
#include <engine/graphics/Image.h>
#include <algorithm>
#include <cmath>
#include <string_view>

namespace game {
using engine::Color;
using engine::Key;
namespace {
// A single layout transform is shared by rendering and mouse hit testing.
struct Layout {
    float scale, x, y;
    explicit Layout(engine::Vec2 size) {
        scale = std::max(0.01f, std::min(size.x / 1280.0f, size.y / 720.0f));
        x = (size.x - 1120 * scale) * 0.5f;
        y = (size.y - 600 * scale) * 0.5f;
    }
    engine::Rect Box(float bx, float by, float w, float h) const {
        return {std::round(x + bx * scale), std::round(y + by * scale), w * scale, h * scale};
    }
    engine::Rect Primary() const { return Box(564, 474, 492, 58); }
    engine::Rect Menu() const { return Box(564, 542, 492, 30); }
};
std::string Clock(int seconds) {
    seconds = std::max(0, seconds);
    return std::to_string(seconds / 60) + ":" + (seconds % 60 < 10 ? "0" : "") +
           std::to_string(seconds % 60);
}
}

bool ResultScene::ReturnsToMenu() const {
    return !m_summary.hasStats || (m_outcome == Outcome::Win && m_summary.stage == config::StageId::LastForest);
}
const char* ResultScene::ActionLabel() const {
    return ReturnsToMenu() ? "KEMBALI KE MENU" : m_outcome == Outcome::Win ? "LANJUT PATROLI" : "COBA SEKALI LAGI";
}
const char* ResultScene::ActionHint() const {
    return ReturnsToMenu() ? "Enter: menu" : m_outcome == Outcome::Win ? "Enter: lanjut" : "Enter: ulangi";
}
void ResultScene::OnEnter() {
    m_time = 0;
    m_hovered = 0;
    App().GetWindow().SetTitle(std::string(config::WindowTitle) + " - Laporan Patroli | " + ActionHint());
    App().GetWindow().SetClearColor(Color::FromHex(0x0C1C18));
    const auto tree = engine::Image::LoadFromFile("assets/environment/Tree.png");
    m_treeSource = tree.GetContentBounds(16);
    m_tree = std::make_unique<engine::Texture>(tree);
    const auto ranger = engine::Image::LoadFromFile("assets/characters/ranger/back_1.png");
    m_rangerSource = ranger.GetContentBounds(16);
    m_ranger = std::make_unique<engine::Texture>(ranger);
    m_sfx = std::make_unique<SoundBank>(App().GetAudio());
    m_sfx->Load({"win", "lose", "ui_click", "ui_hover", "stage_clear_star"});
    m_stars = m_summary.hasStats ? CalculateStars(m_outcome == Outcome::Win,
        m_summary.forestPercent, m_summary.rescued, m_summary.totalAnimals) : 0;
    m_sfx->Play(m_outcome == Outcome::Win ? "win" : "lose", config::sound::Result);
}
void ResultScene::Activate(bool menu) {
    m_sfx->Play("ui_click", config::sound::UiClick);
    if (menu || ReturnsToMenu()) App().GetScenes().SwitchTo(std::make_unique<MenuScene>());
    else {
        auto stage = m_summary.stage;
        if (m_outcome == Outcome::Win) stage = static_cast<config::StageId>(static_cast<int>(stage) + 1);
        App().GetScenes().SwitchTo(std::make_unique<GameplayScene>(stage));
    }
}
void ResultScene::Update(float dt) {
    m_time += dt;
    const int reveal = std::min(m_stars, std::max(0, static_cast<int>((m_time - 0.4f) / 0.3f)));
    if (reveal > m_revealedStars) {
        m_revealedStars = reveal;
        m_sfx->Play("stage_clear_star", 0.7f);
    }
    const auto& input = App().GetInput();
    const Layout layout(App().GetWindow().GetSize());
    const auto mouse = input.GetMousePosition();
    const int hovered = layout.Primary().Contains(mouse) ? 1 :
        (!ReturnsToMenu() && layout.Menu().Contains(mouse) ? 2 : 0);
    if (hovered && hovered != m_hovered) m_sfx->Play("ui_hover", config::sound::UiHover);
    m_hovered = hovered;
    if (input.IsKeyPressed(Key::Escape)) Activate(true);
    else if (input.IsKeyPressed(Key::Enter) || input.IsKeyPressed(Key::NumpadEnter) || input.IsKeyPressed(Key::Space)) Activate(false);
    else if (input.IsMousePressed(engine::MouseButton::Left) && hovered) Activate(hovered == 2);
}
void ResultScene::Draw(float /*alpha*/) {
    const auto size = App().GetWindow().GetSize();
    const Layout layout(size);
    const float s = layout.scale;
    const bool win = m_outcome == Outcome::Win;
    const bool timeout = !win && m_summary.reason == "WAKTU HABIS";
    const bool completed = win && m_summary.hasStats && m_summary.stage == config::StageId::LastForest;
    const Color paper = Color::FromHex(0xE8DCBB), ink = Color::FromHex(0x283C32);
    const Color faded = Color::FromHex(0x73765F), rule = Color::FromHex(0xC8BE9E);
    const Color cream = Color::FromHex(0xF8EDCE);
    const Color accent = Color::FromHex(win ? 0x456C48 : 0xA24932);
    engine::Camera2D camera;
    camera.SetViewportSize(size);
    camera.SetPosition(size * 0.5f);
    auto& batch = App().GetSpriteBatch();
    batch.Begin(camera.GetViewProjection());
    auto rect = [&](float x, float y, float w, float h, Color color) { batch.DrawRect(layout.Box(x, y, w, h), color); };
    auto text = [&](std::string_view value, float x, float y, float scale, Color color) {
        const auto box = layout.Box(x, y, 0, 0);
        // Whole framebuffer pixels preserve the thin glyph strokes in small windows.
        art::DrawPixelText(batch, value, box.x, box.y, std::max(1.0f, std::round(scale * s)), color);
    };
    // Subtle cartographic grid behind the field report.
    for (int i = 0; i < 34; ++i) {
        const float x = size.x * i / 33;
        batch.DrawRect({x, 0, s, size.y}, Color::FromHex(0xA2B48B, 9));
    }
    for (int i = 0; i < 21; ++i)
        batch.DrawRect({0, size.y * i / 20, size.x, s}, Color::FromHex(0xA2B48B, 9));
    text("BURN THE WO / KAWASAN KONSERVASI", 0, -27, 1.5f, Color::FromHex(0xA5B29A));
    text("CATATAN PENJAGA HUTAN", 880, -27, 1.5f, Color::FromHex(0xA5B29A));
    rect(8, 10, 1112, 594, Color::FromHex(0x050E0B, 150));
    rect(0, 0, 500, 600, Color::FromHex(0x1A3028));
    // The left panel is a small scene: distant canopy, scorched trunks, the ranger watching.
    for (int i = 0; i < 42; ++i) {
        const float t = i / 41.0f;
        rect(0, 130 + i * 9.0f, 500, 10, Color{
            static_cast<std::uint8_t>((win ? 33 : 60) - t * 20),
            static_cast<std::uint8_t>((win ? 65 : 60) - t * 24),
            static_cast<std::uint8_t>(43 - t * 17), 255});
    }
    auto tree = [&](float x, float bottom, float height, Color tint) {
        const float width = height * m_treeSource.w / m_treeSource.h;
        batch.Draw(*m_tree, layout.Box(x, bottom - height, width, height), m_treeSource, tint);
    };
    for (int i = 0; i < 6; ++i) tree(10 + i * 75.0f, 375, 130 + (i % 3) * 27.0f, Color::FromHex(0x51614B));
    rect(0, 366, 500, 140, Color::FromHex(0x152820));
    for (int i = 0; i < 4; ++i) {
        const float x = 35 + i * 130.0f;
        if (win) tree(x, 441, 150 + (i % 2) * 35.0f, Color::FromHex(0x87956B));
        else {
            rect(x, 280 + (i % 2) * 45.0f, 13, 170 - (i % 2) * 45.0f, Color::FromHex(0x101C18));
            for (int j = 0; j < 5; ++j) rect(x - 7 * j, 330.0f - 7 * j, 9, 11, Color::FromHex(0x101C18));
            rect(x + 10, 358, 22, 8, Color::FromHex(0x101C18));
        }
    }
    for (int i = 0; i < 18; ++i) {
        const float p = std::fmod(i * 0.127f + m_time * 0.025f, 1.0f);
        rect(22.0f + static_cast<float>(i * 79 % 450), 455 - p * 230, 2, 3,
             Color::FromHex(win ? 0xD5CE82 : 0xE9A262, static_cast<std::uint8_t>(110 * std::sin(p * 3.14159f))));
    }
    // An uneven footpath leads into the forest.
    for (int i = 0; i < 9; ++i) rect(220 - i * 7.0f, 384 + i * 14.0f, 27 + i * 14.0f, 15, Color::FromHex(0x394132));
    rect(208, 474, 88, 10, Color::FromHex(0x0B1913));
    batch.Draw(*m_ranger, layout.Box(219, 350, 126 * m_rangerSource.w / m_rangerSource.h, 126), m_rangerSource);
    rect(0, 506, 500, 94, Color::FromHex(0x13271F));
    text("CATATAN DARI LAPANGAN", 32, 526, 1.5f, Color::FromHex(0xA2B08E));
    text(win ? "SETIAP POHON BERARTI." : "HUTAN INI MASIH MEMBUTUHKANMU.", 32, 552, 2, cream);
    rect(32, 28, 33, 35, Color::FromHex(0x8E9C6D));
    rect(46, 36, 5, 22, cream); rect(39, 42, 19, 5, cream); rect(42, 37, 13, 5, cream);
    text("POS PENJAGA", 80, 31, 2, cream);
    text("LAPORAN AKHIR PATROLI", 80, 53, 1.3f, Color::FromHex(0xA9B69B));
    text("0" + std::to_string(static_cast<int>(m_summary.stage) + 1), 32, 97, 5, cream);
    text(config::GetStage(m_summary.stage).name, 110, 116, 2, cream);
    // The report is a single sheet with a folded corner and restrained print texture.
    rect(500, 0, 620, 600, paper);
    rect(500, 0, 7, 600, Color::FromHex(0xBEB18C));
    for (int i = 0; i < 150; ++i) rect(516.0f + static_cast<float>(i * 137 % 586), 8.0f + static_cast<float>(i * 73 % 583), 2, 1, Color::FromHex(0x8B7956, 17));
    for (int i = 0; i < 24; ++i) rect(1096.0f + i, 0, 24.0f - i, i + 1.0f, Color::FromHex(0xBDB18F));
    text("HASIL OPERASI", 564, 33, 1.5f, faded);
    text(win ? "SELESAI" : "BELUM TUNTAS", win ? 972.0f : 910.0f, 33, 1.5f, accent);
    rect(564, 61, 492, 1, rule);
    const char* title = win ? (completed ? "HUTAN SELAMAT" : "PATROLI TUNTAS") : timeout ? "WAKTU HABIS" : "PATROLI TERHENTI";
    const float titleScale = std::min(4.5f, 492.0f / (6.0f * static_cast<float>(std::string_view(title).size())));
    text(title, 564, 88, titleScale, ink);
    text(win ? "KERJA KERASMU MENJAGA HUTAN TETAP HIDUP." : timeout ? "WAKTU BERAKHIR. PERJUANGAN BELUM SELESAI." : m_summary.reason,
         564, 135, 1.5f, accent);
    text(win ? "SETIAP USAHA MEMBUAT PERBEDAAN." : "AMBIL NAPAS. SUSUN LANGKAH. KEMBALI JAGA HUTAN.",
         564, 157, 1.3f, faded);
    rect(564, 191, 492, 1, rule);
    if (m_summary.hasStats) {
        text("HUTAN TERSISA", 564, 211, 1.5f, ink);
        text(std::to_string(m_summary.forestPercent) + "%", 943, 203, 3, ink);
        rect(564, 240, 492, 12, Color::FromHex(0xD0C5A5));
        rect(564, 240, 492 * std::clamp(m_summary.forestPercent / 100.0f, 0.0f, 1.0f), 12, Color::FromHex(0x627D50));
        text("LAHAN SAWIT", 564, 266, 1.5f, faded);
        text(std::to_string(m_summary.sawitPercent) + "%", 995, 266, 1.5f, accent);
        rect(564, 296, 492, 1, rule);
        const char* labels[] = {"API DIPADAMKAN", "SATWA SELAMAT", "SISA WAKTU"};
        const std::string values[] = {std::to_string(m_summary.firesPutOut),
            std::to_string(m_summary.rescued) + "/" + std::to_string(m_summary.rescueTarget), Clock(m_summary.secondsLeft)};
        for (int i = 0; i < 3; ++i) {
            const float x = 564 + i * 174.0f;
            text(labels[i], x, 316, 1.2f, faded);
            text(values[i], x, 342, 3, i == 2 && timeout ? accent : ink);
            if (i < 2) rect(x + 152, 317, 1, 53, rule);
        }
    } else {
        text("PATROLI BERAKHIR", 564, 235, 2, ink);
        text("RINGKASAN LAPANGAN TIDAK TERSEDIA.", 564, 269, 1.5f, faded);
    }
    if (m_summary.hasStats) {
        text("PENILAIAN", 564, 382, 1.2f, faded);
        for (int i = 0; i < 3; ++i) {
            const auto color = i < m_revealedStars ? Color::FromHex(0xAD791F) : rule;
            const float x = 929.0f + i * 44;
            constexpr const char* shape[] = {
                "     #     ", "    ###    ", "    ###    ", "###########",
                " ######### ", "  #######  ", "  #######  ", "  ### ###  ",
                " ###   ### ", " ##     ## "
            };
            for (int row = 0; row < 10; ++row) {
                for (int col = 0; col < 11; ++col) {
                    if (shape[row][col] == '#') rect(x + col*2, 378 + row*2, 2, 2, color);
                }
            }
        }
    }
    rect(564, 406, 492, 1, rule);
    text(win ? "LANGKAH BERIKUTNYA" : "UNTUK PATROLI BERIKUTNYA", 564, 411, 1.2f, faded);
    text(win ? (completed ? "TERIMA KASIH SUDAH MENJAGA HUTAN." : "SIAPKAN AIR. TANTANGAN BARU MENUNGGU.") :
         "PADAMKAN API TERDEKAT, LALU DEKATI SATWA.", 564, 432, 1.4f, ink);
    const bool hover = layout.Primary().Contains(App().GetInput().GetMousePosition());
    rect(564, 479, 492, 58, Color::FromHex(0xB7AB89));
    batch.DrawRect(layout.Primary(), hover ? Color::FromHex(0x48664A) : ink);
    text(ActionLabel(), 586, 495, 2, cream);
    text("ENTER", 982, 498, 1.3f, Color::FromHex(0xBDC8A7));
    if (!ReturnsToMenu()) {
        const bool menuHover = layout.Menu().Contains(App().GetInput().GetMousePosition());
        text("ESC / KEMBALI KE MENU", 700, 551, 1.4f, menuHover ? ink : faded);
        if (menuHover) rect(700, 566, 166, 1, ink);
    }
    text("JAGA HUTAN. JAGA KEHIDUPAN.", 0, 624, 1.3f, Color::FromHex(0x9AA98D));
    text("TIM DAPUR TEDDUY", 1003, 624, 1.3f, Color::FromHex(0x9AA98D));
    // Brief fade-in lets the report settle after the stage ends.
    if (m_time < 0.35f) batch.DrawRect({0, 0, size.x, size.y}, Color::FromHex(0x0C1C18,
        static_cast<std::uint8_t>(220 * (1 - m_time / 0.35f))));
    batch.End();
}
} // namespace game
