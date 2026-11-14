#include "audio/SoundBank.h"

#include <engine/core/Log.h>

#include <exception>
#include <filesystem>

namespace game {

namespace {
const char* const SfxFolder = "assets/audio/sfx/";
constexpr int MaxVariants = 16;
} // namespace

void SoundBank::Load(std::initializer_list<const char*> names) {
    namespace fs = std::filesystem;
    for (const char* name : names) {
        Entry entry;
        try {
            // name_1.wav, name_2.wav, ... until one is missing
            for (int i = 1; i <= MaxVariants; ++i) {
                const fs::path path = std::string(SfxFolder) + name + "_" + std::to_string(i) + ".wav";
                if (!fs::exists(path)) break;
                entry.variants.push_back(m_audio.LoadSound(path));
            }
            // otherwise a single name.wav
            if (entry.variants.empty()) {
                const fs::path path = std::string(SfxFolder) + name + ".wav";
                if (fs::exists(path)) entry.variants.push_back(m_audio.LoadSound(path));
            }
        } catch (const std::exception& e) {
            engine::LogError("Sound '%s' skipped: %s", name, e.what());
            continue;
        }
        if (entry.variants.empty()) {
            engine::LogError("Sound '%s' not found in %s", name, SfxFolder);
            continue;
        }
        m_entries[name] = std::move(entry);
    }
}

std::shared_ptr<const engine::Sound> SoundBank::Pick(const std::string& name) {
    const auto it = m_entries.find(name);
    if (it == m_entries.end()) return nullptr;
    Entry& entry = it->second;
    std::size_t index = 0;
    if (entry.variants.size() > 1) {
        // Draw from the other variants only, so the same one never plays twice in a row.
        std::uniform_int_distribution<std::size_t> pick(0, entry.variants.size() - 2);
        index = pick(m_rng);
        if (index >= entry.last) ++index;
    }
    entry.last = index;
    return entry.variants[index];
}

engine::AudioSystem::VoiceId SoundBank::Play(const std::string& name, float volume) {
    const auto sound = Pick(name);
    return sound ? m_audio.Play(sound, volume, false) : engine::AudioSystem::InvalidVoice;
}

engine::AudioSystem::VoiceId SoundBank::Loop(const std::string& name, float volume) {
    const auto sound = Pick(name);
    return sound ? m_audio.Play(sound, volume, true) : engine::AudioSystem::InvalidVoice;
}

} // namespace game
