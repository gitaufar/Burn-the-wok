#pragma once

#include <engine/audio/Audio.h>

#include <cstddef>
#include <initializer_list>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <vector>

namespace game {

// Game-side helper: loads sound effects from assets/audio/sfx by name and plays them.
// A name may have several variants on disk (name_1.wav, name_2.wav, ...) or one plain
// name.wav. Play() picks a random variant and never repeats the one it just played, so
// sounds that happen often (footsteps, clicks) do not sound mechanical.
//
// A missing file is logged and skipped: the game keeps running, just without that sound.
class SoundBank {
public:
    explicit SoundBank(engine::AudioSystem& audio) : m_audio(audio) {}

    void Load(std::initializer_list<const char*> names);
    bool Has(const std::string& name) const { return m_entries.count(name) > 0; }

    // One-shot. volume 0..1. Returns the voice, or InvalidVoice (silent / unknown name).
    engine::AudioSystem::VoiceId Play(const std::string& name, float volume = 1.0f);
    // Looping sound: the caller stops it with AudioSystem::Stop(voice) when it is no longer wanted.
    engine::AudioSystem::VoiceId Loop(const std::string& name, float volume = 1.0f);

private:
    struct Entry {
        std::vector<std::shared_ptr<const engine::Sound>> variants;
        std::size_t last = 0; // variant played most recently
    };
    std::shared_ptr<const engine::Sound> Pick(const std::string& name);

    engine::AudioSystem& m_audio;
    std::map<std::string, Entry> m_entries;
    std::mt19937 m_rng{std::random_device{}()};
};

} // namespace game
