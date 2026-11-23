#pragma once

#include "audio/SoundBank.h"
#include <engine/core/Scene.h>
#include <memory>
#include <string>

namespace game {
class StoreScene final : public engine::Scene {
public:
    void OnEnter() override;
    void OnExit() override;
    void Update(float dt) override;
    void Draw(float alpha) override;

private:
    void Purchase();
    int m_selected = 0;
    float m_poster = 0;
    float m_notice = 0;
    bool m_error = false;
    std::string m_message;
    std::unique_ptr<SoundBank> m_sfx;
    engine::AudioSystem::VoiceId m_music = engine::AudioSystem::InvalidVoice;
};
}
