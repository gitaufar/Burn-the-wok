#pragma once

#include "audio/SoundBank.h"

#include <engine/audio/Audio.h>
#include <engine/core/Scene.h>
#include <engine/graphics/Texture.h>
#include <engine/math/Rect.h>

#include <memory>

namespace game {

// Main menu, stage selection, and progress reset confirmation.
class MenuScene : public engine::Scene {
public:
    void OnEnter() override;
    void OnExit() override;
    void Update(float dt) override;
    void Draw(float alpha) override;

private:
    std::unique_ptr<engine::Texture> m_logo;
    std::unique_ptr<engine::Texture> m_glow;
    engine::Rect m_logoSource;
    float m_time = 0.0f;
    std::unique_ptr<SoundBank> m_sfx;
    int m_focus = 0;
    int m_hovered = -1;
    enum class Page { Main, Stages, Reset };
    Page m_page = Page::Main;
    bool m_resetFailed = false;
    void Select(int index);
    std::shared_ptr<const engine::Sound> m_music;
    engine::AudioSystem::VoiceId m_voice = engine::AudioSystem::InvalidVoice;
};

} // namespace game
