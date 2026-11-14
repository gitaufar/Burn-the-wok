#pragma once

#include "audio/SoundBank.h"

#include <engine/audio/Audio.h>
#include <engine/core/Scene.h>
#include <engine/graphics/Texture.h>

#include <memory>
#include <string>
#include <vector>

namespace game {

// Opening cutscene shown before the menu: illustrated slides with narration, read from
// assets/intro/narration.txt (so the team can edit the text without recompiling).
//
// Flow of one slide:  a beat of picture alone -> pages of narration (typed out, then held) -> next slide.
// After the last page the picture and the music fade to black, then the menu opens.
//   Click / Space / Enter : finish typing the page, or go to the next page
//   Esc                   : skip the whole intro
class IntroScene : public engine::Scene {
public:
    void OnEnter() override;
    void OnExit() override;
    void Update(float dt) override;
    void Draw(float alpha) override;

private:
    enum class Phase {
        Title,      // a short beat: just the picture, before the narration starts
        Reading,    // narration pages
        Transition, // crossfade from the previous slide to the current one
        Ending      // fade to black, then switch to the menu
    };

    struct Slide {
        std::unique_ptr<engine::Texture> image;
        float imageWidth = 1.0f;
        float imageHeight = 1.0f;
        std::vector<std::string> pages;
        float time = 0.0f;   // drives the slow camera drift
        float phase = 0.0f;  // different drift direction per slide
    };

    void LoadScript(const std::string& path);
    void RequestAdvance();
    void NextPage();
    void BeginEnding(float fadeSeconds);
    void Finish();

    void DrawSlideImage(int index, float alpha) const;
    void DrawGradient(float alpha) const;
    void DrawPage() const;
    void DrawHints(float alpha) const;

    int PagesBefore(int slide) const; // for the progress bar
    int TotalPages() const;

    std::vector<Slide> m_slides;
    std::unique_ptr<engine::Texture> m_gradient; // 1x256 vertical ramp: clear (top) to dark (bottom)
    int m_slide = 0;
    int m_from = -1;       // previous slide while crossfading
    int m_page = 0;
    Phase m_phase = Phase::Title;

    float m_time = 0.0f;        // since the scene started (fade-in at the start)
    float m_slideTime = 0.0f;   // since the current slide started
    float m_pageTime = 0.0f;    // since the current page started
    float m_transition = 0.0f;  // 0..1 crossfade progress
    float m_panel = 0.0f;       // eased 0..1 strength of the dark gradient behind the text
    float m_endTimer = 0.0f;
    float m_endFade = 1.0f;     // seconds of the closing fade
    bool m_ending = false;
    bool m_failed = false;      // narration file could not be read -> go straight to the menu

    std::unique_ptr<SoundBank> m_sfx;
    std::shared_ptr<const engine::Sound> m_music;
    engine::AudioSystem::VoiceId m_voice = engine::AudioSystem::InvalidVoice;
};

} // namespace game
