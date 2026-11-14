#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <vector>

namespace engine {

// A decoded sound: interleaved stereo signed 16-bit samples at the mixer's sample rate.
// Created by AudioSystem::LoadSound; immutable afterwards, so it can be shared freely.
class Sound {
public:
    int GetFrameCount() const { return static_cast<int>(m_samples.size() / 2); }
    float GetDurationSeconds() const {
        return m_sampleRate > 0 ? static_cast<float>(GetFrameCount()) / static_cast<float>(m_sampleRate) : 0.0f;
    }

private:
    friend class AudioSystem;
    std::vector<std::int16_t> m_samples;
    int m_sampleRate = 44100;
};

// Our own tiny audio mixer on top of the OS audio device opened through SDL2 (core only,
// no SDL_mixer). A fixed number of voices are summed in the audio callback. If no audio
// device is available the system stays silent: every call below becomes a harmless no-op.
//
// Thread safety: the device callback runs on its own thread. Public functions take SDL's
// device lock, so call them from the game thread as usual.
class AudioSystem {
public:
    using VoiceId = int;
    static constexpr VoiceId InvalidVoice = -1;

    AudioSystem();
    ~AudioSystem();
    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;

    bool IsAvailable() const { return m_device != 0; }

    // Reads a PCM WAV file (8 or 16 bit, mono or stereo, any sample rate) and converts it
    // to the mixer format. Throws std::runtime_error when the file is missing or unsupported.
    std::shared_ptr<const Sound> LoadSound(const std::filesystem::path& path) const;

    // Starts a sound. volume is 0..1. A looping voice plays until Stop() is called.
    // Returns InvalidVoice when silent or when all voices are busy.
    VoiceId Play(std::shared_ptr<const Sound> sound, float volume = 1.0f, bool loop = false);
    void SetVoiceVolume(VoiceId voice, float volume);
    void Stop(VoiceId voice);
    void StopAll();
    bool IsPlaying(VoiceId voice) const;
    void SetMasterVolume(float volume);

    // Called from the audio thread only. Fills `frames` stereo frames into `out`.
    void MixInto(std::int16_t* out, int frames);

private:
    static constexpr int VoiceCount = 16;
    struct Voice {
        std::shared_ptr<const Sound> sound;
        std::size_t frame = 0;
        float volume = 1.0f;
        bool loop = false;
        bool active = false;
        std::uint32_t generation = 0; // makes a stale VoiceId from a finished sound harmless
    };

    int FindVoice(VoiceId id) const; // index or -1; caller holds the lock

    unsigned int m_device = 0; // SDL_AudioDeviceID, 0 = no device
    int m_sampleRate = 44100;
    float m_masterVolume = 1.0f;
    Voice m_voices[VoiceCount];
};

} // namespace engine
