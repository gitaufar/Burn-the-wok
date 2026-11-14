#include "scenes/IntroScene.h"

#include "GameConfig.h"
#include "art/PixelText.h"
#include "scenes/MenuScene.h"

#include <engine/core/Application.h>
#include <engine/core/Log.h>
#include <engine/graphics/Camera2D.h>
#include <engine/graphics/Image.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

namespace game {

using engine::Color;
using engine::Key;
using engine::Rect;
using engine::Vec2;

namespace {

const char* const ScriptPath = "assets/intro/narration.txt";
const char* const MusicPath = "assets/audio/music/sad_intro.wav";

constexpr float TitleDuration = 2.6f;     // seconds of picture alone before the narration starts, per slide
constexpr float FadeInDuration = 2.2f;    // from black at the very start
constexpr float SlideTransition = 1.9f;   // crossfade between slides
constexpr float EndFadeDuration = 3.2f;   // closing fade to black
constexpr float SkipFadeDuration = 0.9f;  // when the player presses Esc
constexpr float EndHold = 0.9f;           // stay black for a moment before the menu
constexpr float CharsPerSecond = 44.0f;   // typewriter speed
constexpr float MusicVolume = 0.8f;

float Clamp01(float v) { return std::clamp(v, 0.0f, 1.0f); }
float Smooth(float v) {
    v = Clamp01(v);
    return v * v * (3.0f - 2.0f * v);
}
std::uint8_t Alpha(float v) { return static_cast<std::uint8_t>(Clamp01(v) * 255.0f + 0.5f); }

// Reading time after a page has been typed out: longer pages are held longer.
float HoldTime(std::size_t length) { return 1.2f + 0.009f * static_cast<float>(length); }
float TypeTime(std::size_t length) { return static_cast<float>(length) / CharsPerSecond; }

// The pixel font only has A-Z, 0-9 and a little punctuation, and the source files are ASCII:
// turn UTF-8 text into upper-case ASCII (n-tilde -> N, dashes -> '-', fancy quotes -> plain).
std::string ToAscii(const std::string& in) {
    std::string out;
    for (std::size_t i = 0; i < in.size();) {
        const unsigned char c = static_cast<unsigned char>(in[i]);
        if (c < 0x80) {
            out.push_back(static_cast<char>(std::toupper(c)));
            ++i;
            continue;
        }
        std::size_t length = c >= 0xF0 ? 4 : c >= 0xE0 ? 3 : c >= 0xC0 ? 2 : 1;
        length = std::min(length, in.size() - i);
        if (c == 0xC3 && length == 2) {
            const unsigned char d = static_cast<unsigned char>(in[i + 1]);
            if (d == 0xB1 || d == 0x91) out.push_back('N'); // n with tilde
        } else if (c == 0xE2 && length == 3) {
            const unsigned char d = static_cast<unsigned char>(in[i + 1]);
            const unsigned char e = static_cast<unsigned char>(in[i + 2]);
            if (d == 0x80 && (e == 0x93 || e == 0x94)) out.push_back('-');
            else if (d == 0x80 && (e == 0x98 || e == 0x99)) out.push_back('\'');
            else if (d == 0x80 && (e == 0x9C || e == 0x9D)) out.push_back('"');
        }
        i += length; // anything else is dropped
    }
    return out;
}

std::string Trim(const std::string& s) {
    std::size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}

std::vector<std::string> Wrap(const std::string& text, int maxChars) {
    std::vector<std::string> lines;
    std::string current;
    std::size_t i = 0;
    while (i < text.size()) {
        const std::size_t end = text.find(' ', i);
        const std::string word = text.substr(i, end == std::string::npos ? std::string::npos : end - i);
        if (!current.empty() && static_cast<int>(current.size() + 1 + word.size()) > maxChars) {
            lines.push_back(current);
            current.clear();
        }
        if (!current.empty()) current.push_back(' ');
        current += word;
        if (end == std::string::npos) break;
        i = end + 1;
    }
    if (!current.empty()) lines.push_back(current);
    return lines;
}

struct TextMetrics {
    int scale;         // pixel-font scale (an integer keeps every font pixel crisp)
    float cell;        // width of one character cell
    float lineHeight;
    int maxChars;      // characters per line
    float x0;          // left edge of the text block
};

TextMetrics MakeMetrics(Vec2 size) {
    const float k = std::min(size.x / 1280.0f, size.y / 720.0f);
    TextMetrics m;
    m.scale = std::max(2, static_cast<int>(std::floor(3.0f * k + 0.5f)));
    m.cell = 6.0f * static_cast<float>(m.scale);
    m.lineHeight = 10.0f * static_cast<float>(m.scale);
    m.maxChars = std::max(12, static_cast<int>(size.x * 0.84f / m.cell));
    m.x0 = (size.x - static_cast<float>(m.maxChars) * m.cell) * 0.5f;
    return m;
}

float TextWidth(std::size_t chars, float scale) {
    return chars == 0 ? 0.0f : static_cast<float>(chars) * 6.0f * scale - scale;
}

// Text with a soft drop shadow so it stays readable on any picture.
void DrawShadowedText(engine::SpriteBatch& batch, std::string_view text, float x, float y,
                      float scale, Color color) {
    art::DrawPixelText(batch, text, x + scale, y + scale, scale,
                       Color::FromHex(0x050403, static_cast<std::uint8_t>(color.a * 0.8f)));
    art::DrawPixelText(batch, text, x, y, scale, color);
}

engine::Camera2D ScreenCamera(Vec2 size) {
    engine::Camera2D camera;
    camera.SetViewportSize(size);
    camera.SetPosition(size * 0.5f);
    return camera;
}

} // namespace

void IntroScene::LoadScript(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("cannot open " + path);
    }
    std::string line;
    bool first = true;
    while (std::getline(file, line)) {
        if (first && line.size() >= 3 && line.compare(0, 3, "\xEF\xBB\xBF") == 0) line.erase(0, 3); // UTF-8 BOM
        first = false;
        line = Trim(line);
        if (line.empty() || line[0] == '#') continue;

        if (line.rfind("[scene]", 0) == 0) {
            const std::string rest = line.substr(7);
            const std::size_t bar = rest.find('|');
            const std::string imagePath = Trim(bar == std::string::npos ? rest : rest.substr(0, bar));
            // Text after '|' is only a label for whoever edits the file; it is not shown in the game.
            Slide slide;
            slide.phase = 1.9f * static_cast<float>(m_slides.size());
            const engine::Image image = engine::Image::LoadFromFile(imagePath);
            slide.imageWidth = static_cast<float>(image.GetWidth());
            slide.imageHeight = static_cast<float>(image.GetHeight());
            slide.image = std::make_unique<engine::Texture>(image);
            m_slides.push_back(std::move(slide));
        } else if (!m_slides.empty()) {
            const std::string page = ToAscii(line);
            if (!page.empty()) m_slides.back().pages.push_back(page);
        }
    }
    if (m_slides.empty()) {
        throw std::runtime_error("no [scene] found in " + path);
    }
}

void IntroScene::OnEnter() {
    App().GetWindow().SetTitle(std::string(config::WindowTitle) +
                               " - Intro | Klik/Spasi: lanjut, Esc: lewati");
    App().GetWindow().SetClearColor(Color::Black());

    // Smooth shading textures (stretched over the screen) avoid the seams that stacked strips would show.
    engine::Image ramp(1, 256);
    for (int y = 0; y < 256; ++y) {
        ramp.SetPixel(0, y, Color::FromHex(0x040302, Alpha(0.9f * Smooth((static_cast<float>(y) + 0.5f) / 256.0f))));
    }
    m_gradient = std::make_unique<engine::Texture>(ramp);

    try {
        LoadScript(ScriptPath);
    } catch (const std::exception& e) {
        engine::LogError("Intro skipped: %s", e.what());
        m_failed = true;
        return;
    }
    try {
        m_music = App().GetAudio().LoadSound(MusicPath);
        m_voice = App().GetAudio().Play(m_music, 0.0f, true); // faded in from Update()
    } catch (const std::exception& e) {
        engine::LogError("Intro music unavailable: %s", e.what()); // the intro still works in silence
    }

    m_sfx = std::make_unique<SoundBank>(App().GetAudio());
    m_sfx->Load({"ui_click"});
}

void IntroScene::OnExit() {
    App().GetAudio().Stop(m_voice);
}

int IntroScene::PagesBefore(int slide) const {
    int total = 0;
    for (int i = 0; i < slide; ++i) total += static_cast<int>(m_slides[static_cast<std::size_t>(i)].pages.size());
    return total;
}

int IntroScene::TotalPages() const {
    return std::max(1, PagesBefore(static_cast<int>(m_slides.size())));
}

void IntroScene::BeginEnding(float fadeSeconds) {
    if (m_ending) return;
    m_ending = true;
    m_endFade = fadeSeconds;
    m_endTimer = 0.0f;
    m_phase = Phase::Ending;
}

void IntroScene::Finish() {
    App().GetScenes().SwitchTo(std::make_unique<MenuScene>());
}

void IntroScene::RequestAdvance() {
    if (m_phase == Phase::Title) {
        m_slideTime = std::max(m_slideTime, TitleDuration); // skip the title card
    } else if (m_phase == Phase::Reading) {
        const std::size_t length = m_slides[static_cast<std::size_t>(m_slide)].pages[static_cast<std::size_t>(m_page)].size();
        const float typed = TypeTime(length);
        if (m_pageTime < typed) {
            m_pageTime = typed; // first click: show the whole page
        } else {
            m_pageTime = std::max(m_pageTime, typed + HoldTime(length) - 0.35f); // second click: move on
        }
    }
}

void IntroScene::NextPage() {
    const Slide& slide = m_slides[static_cast<std::size_t>(m_slide)];
    if (m_page + 1 < static_cast<int>(slide.pages.size())) {
        ++m_page;
        m_pageTime = 0.0f;
    } else if (m_slide + 1 < static_cast<int>(m_slides.size())) {
        m_from = m_slide;
        ++m_slide;
        m_page = 0;
        m_pageTime = 0.0f;
        m_slideTime = 0.0f;
        m_transition = 0.0f;
        m_phase = Phase::Transition;
    } else {
        BeginEnding(EndFadeDuration);
    }
}

void IntroScene::Update(float dt) {
    if (m_failed) {
        Finish();
        return;
    }
    const engine::Input& input = App().GetInput();
    m_time += dt;
    m_slideTime += dt;
    m_slides[static_cast<std::size_t>(m_slide)].time += dt;
    if (m_from >= 0) m_slides[static_cast<std::size_t>(m_from)].time += dt;

    if (input.IsKeyPressed(Key::Escape)) {
        BeginEnding(SkipFadeDuration);
    } else if (input.IsKeyPressed(Key::Enter) || input.IsKeyPressed(Key::NumpadEnter) ||
               input.IsKeyPressed(Key::Space) || input.IsMousePressed(engine::MouseButton::Left)) {
        if (m_sfx && (m_phase == Phase::Title || m_phase == Phase::Reading)) m_sfx->Play("ui_click", 0.35f);
        RequestAdvance();
    }

    switch (m_phase) {
    case Phase::Title:
        if (m_slideTime >= TitleDuration) {
            m_phase = Phase::Reading;
            m_page = 0;
            m_pageTime = 0.0f;
        }
        break;
    case Phase::Reading: {
        m_pageTime += dt;
        const std::size_t length = m_slides[static_cast<std::size_t>(m_slide)].pages[static_cast<std::size_t>(m_page)].size();
        if (m_pageTime >= TypeTime(length) + HoldTime(length)) NextPage();
        break;
    }
    case Phase::Transition:
        m_transition += dt / SlideTransition;
        if (m_transition >= 1.0f) {
            m_transition = 1.0f;
            m_from = -1;
            m_phase = m_slideTime >= TitleDuration ? Phase::Reading : Phase::Title;
        }
        break;
    case Phase::Ending:
        break;
    }

    // The dark gradient behind the text eases in while reading and out otherwise.
    const float panelTarget = m_phase == Phase::Reading ? 1.0f : 0.0f;
    m_panel += (panelTarget - m_panel) * std::min(1.0f, dt * 4.0f);

    float endProgress = 0.0f;
    if (m_ending) {
        m_endTimer += dt;
        endProgress = Smooth(m_endTimer / m_endFade);
        if (m_endTimer >= m_endFade + EndHold) {
            Finish();
        }
    }
    App().GetAudio().SetVoiceVolume(m_voice, MusicVolume * Smooth(m_time / 3.0f) * (1.0f - endProgress));
}

void IntroScene::DrawSlideImage(int index, float alpha) const {
    const Slide& slide = m_slides[static_cast<std::size_t>(index)];
    if (!slide.image || alpha <= 0.0f) return;
    const Vec2 size = App().GetWindow().GetSize();

    // "Cover" the window, then drift slowly (zoom + pan) so the still picture feels alive.
    const float cover = std::max(size.x / slide.imageWidth, size.y / slide.imageHeight);
    const float zoom = std::min(1.22f, 1.05f + 0.0013f * slide.time);
    const float w = slide.imageWidth * cover * zoom;
    const float h = slide.imageHeight * cover * zoom;
    const float panX = std::sin(slide.time * 0.05f + slide.phase) * (w - size.x) * 0.5f * 0.85f;
    const float panY = std::sin(slide.time * 0.037f + slide.phase * 1.7f) * (h - size.y) * 0.5f * 0.85f;
    const Rect dest{(size.x - w) * 0.5f + panX, (size.y - h) * 0.5f + panY, w, h};
    App().GetSpriteBatch().Draw(*slide.image, dest, Color::White().WithAlpha(Alpha(alpha)));
}

void IntroScene::DrawGradient(float alpha) const {
    if (alpha <= 0.01f) return;
    const Vec2 size = App().GetWindow().GetSize();
    const float top = size.y * 0.48f;
    App().GetSpriteBatch().Draw(*m_gradient, {0.0f, top, size.x, size.y - top}, Color::White().WithAlpha(Alpha(alpha)));
}

void IntroScene::DrawPage() const {
    const Vec2 size = App().GetWindow().GetSize();
    const TextMetrics metrics = MakeMetrics(size);
    const Slide& slide = m_slides[static_cast<std::size_t>(m_slide)];
    const std::string& page = slide.pages[static_cast<std::size_t>(m_page)];
    engine::SpriteBatch& batch = App().GetSpriteBatch();

    const float typeTime = TypeTime(page.size());
    const float end = typeTime + HoldTime(page.size());
    const float a = std::min(Clamp01(m_pageTime / 0.25f), Clamp01((end - m_pageTime) / 0.35f));
    if (a <= 0.0f) return;

    const std::vector<std::string> lines = Wrap(page, metrics.maxChars);
    const float scale = static_cast<float>(metrics.scale);
    const float blockHeight = static_cast<float>(lines.size()) * metrics.lineHeight;
    const float y0 = size.y * 0.915f - blockHeight; // bottom-anchored: the text does not jump while typing
    int remaining = static_cast<int>(m_pageTime * CharsPerSecond);
    for (std::size_t i = 0; i < lines.size(); ++i) {
        const int n = std::clamp(remaining, 0, static_cast<int>(lines[i].size()));
        if (n > 0) {
            DrawShadowedText(batch, std::string_view(lines[i]).substr(0, static_cast<std::size_t>(n)), metrics.x0,
                             y0 + static_cast<float>(i) * metrics.lineHeight, scale, Color::FromHex(0xFFF0CB, Alpha(a)));
        }
        remaining -= static_cast<int>(lines[i].size()) + 1;
    }

    // A small blinking arrow says "click to continue" once the page is fully typed.
    if (m_pageTime >= typeTime) {
        const float blink = 0.5f + 0.5f * std::sin(m_time * 6.0f);
        const float ax = metrics.x0 + static_cast<float>(metrics.maxChars) * metrics.cell - 3.0f * scale;
        const float ay = y0 + blockHeight + scale * 0.5f;
        const Color c = Color::FromHex(0xFFB95A, Alpha(a * (0.35f + 0.65f * blink)));
        for (int row = 0; row < 3; ++row) {
            batch.DrawRect({ax + static_cast<float>(row) * scale, ay + static_cast<float>(row) * scale,
                            static_cast<float>(5 - 2 * row) * scale, scale}, c);
        }
    }
}

void IntroScene::DrawHints(float alpha) const {
    if (alpha <= 0.01f) return;
    const Vec2 size = App().GetWindow().GetSize();
    const TextMetrics metrics = MakeMetrics(size);
    const float scale = std::max(1.0f, static_cast<float>(metrics.scale) - 1.0f);
    const std::string hint = "KLIK / SPASI: LANJUT    ESC: LEWATI";
    const float x = metrics.x0 + static_cast<float>(metrics.maxChars) * metrics.cell - TextWidth(hint.size(), scale);
    art::DrawPixelText(App().GetSpriteBatch(), hint, x, size.y * 0.955f, scale,
                       Color::FromHex(0xE8D9AF, Alpha(alpha * 0.55f)));
}

void IntroScene::Draw(float /*alpha*/) {
    if (m_failed) return;
    const Vec2 size = App().GetWindow().GetSize();
    engine::SpriteBatch& batch = App().GetSpriteBatch();
    engine::Camera2D camera = ScreenCamera(size);
    batch.Begin(camera.GetViewProjection());

    if (m_phase == Phase::Transition && m_from >= 0) {
        DrawSlideImage(m_from, 1.0f);
        DrawSlideImage(m_slide, Smooth(m_transition));
    } else {
        DrawSlideImage(m_slide, 1.0f);
    }
    DrawGradient(m_panel);

    if (m_phase == Phase::Reading) {
        DrawPage();
    }
    DrawHints(Smooth((m_time - 5.0f) / 1.5f) * (m_ending ? 0.0f : 1.0f));

    // Thin progress line at the very bottom edge.
    const int done = PagesBefore(m_slide) + (m_phase == Phase::Reading ? m_page : 0);
    const float progress = m_ending ? 1.0f : static_cast<float>(done) / static_cast<float>(TotalPages());
    batch.DrawRect({0.0f, size.y - 3.0f, size.x, 3.0f}, Color::FromHex(0x000000, 90));
    batch.DrawRect({0.0f, size.y - 3.0f, size.x * progress, 3.0f}, Color::FromHex(0xFFB95A, 200));

    // Black overlay: fade-in at the start, closing fade at the end.
    float black = 1.0f - Smooth(m_time / FadeInDuration);
    if (m_ending) black = std::max(black, Smooth(m_endTimer / m_endFade));
    if (black > 0.0f) {
        batch.DrawRect({0.0f, 0.0f, size.x, size.y}, Color::Black().WithAlpha(Alpha(black)));
    }
    batch.End();
}

} // namespace game
