#include "engine/audio/Audio.h"

#include "engine/core/Log.h"

#include <SDL/SDL.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace engine {

namespace {

// Called by SDL on the audio thread.
void SDLCALL AudioCallback(void* userdata, Uint8* stream, int length) {
    const int frames = length / static_cast<int>(sizeof(std::int16_t) * 2);
    static_cast<AudioSystem*>(userdata)->MixInto(reinterpret_cast<std::int16_t*>(stream), frames);
}

std::uint16_t ReadU16(const std::uint8_t* p) { return static_cast<std::uint16_t>(p[0] | (p[1] << 8)); }
std::uint32_t ReadU32(const std::uint8_t* p) {
    return static_cast<std::uint32_t>(p[0]) | (static_cast<std::uint32_t>(p[1]) << 8) |
           (static_cast<std::uint32_t>(p[2]) << 16) | (static_cast<std::uint32_t>(p[3]) << 24);
}

} // namespace

AudioSystem::AudioSystem() {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        LogError("Audio disabled: %s", SDL_GetError());
        return;
    }
    SDL_AudioSpec want;
    SDL_zero(want);
    want.freq = 44100;
    want.format = AUDIO_S16SYS;
    want.channels = 2;
    want.samples = 2048; // ~46 ms of latency: fine for music and effects
    want.callback = AudioCallback;
    want.userdata = this;

    SDL_AudioSpec have;
    SDL_zero(have);
    // Do not allow SDL to change the format: the mixer assumes interleaved stereo S16.
    m_device = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
    if (m_device == 0) {
        LogError("Audio disabled: could not open an audio device (%s)", SDL_GetError());
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return;
    }
    m_sampleRate = have.freq;
    SDL_PauseAudioDevice(m_device, 0); // start the callback
}

AudioSystem::~AudioSystem() {
    if (m_device != 0) {
        SDL_CloseAudioDevice(m_device); // waits for the callback thread to stop
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }
}

std::shared_ptr<const Sound> AudioSystem::LoadSound(const std::filesystem::path& path) const {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("LoadSound: cannot open " + path.string());
    }
    const std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(file)),
                                          std::istreambuf_iterator<char>());
    if (bytes.size() < 12 || std::memcmp(bytes.data(), "RIFF", 4) != 0 ||
        std::memcmp(bytes.data() + 8, "WAVE", 4) != 0) {
        throw std::runtime_error("LoadSound: not a WAV file: " + path.string());
    }

    int channels = 0, bits = 0, rate = 0;
    const std::uint8_t* data = nullptr;
    std::size_t dataSize = 0;
    for (std::size_t pos = 12; pos + 8 <= bytes.size();) {
        const std::uint8_t* chunk = bytes.data() + pos;
        const std::size_t size = ReadU32(chunk + 4);
        const std::size_t body = pos + 8;
        if (std::memcmp(chunk, "fmt ", 4) == 0 && size >= 16 && body + 16 <= bytes.size()) {
            const std::uint16_t tag = ReadU16(chunk + 8);
            // 1 = PCM, 0xFFFE = WAVE_FORMAT_EXTENSIBLE (we only accept its PCM flavour below).
            if (tag != 1 && tag != 0xFFFE) {
                throw std::runtime_error("LoadSound: only PCM WAV is supported: " + path.string());
            }
            channels = ReadU16(chunk + 10);
            rate = static_cast<int>(ReadU32(chunk + 12));
            bits = ReadU16(chunk + 22);
        } else if (std::memcmp(chunk, "data", 4) == 0) {
            data = bytes.data() + body;
            dataSize = std::min(size, bytes.size() - body);
            break;
        }
        pos = body + size + (size & 1); // chunks are padded to an even size
    }
    if (!data || channels < 1 || channels > 2 || rate <= 0 || (bits != 8 && bits != 16)) {
        throw std::runtime_error("LoadSound: unsupported WAV (need 8/16-bit, mono/stereo PCM): " + path.string());
    }

    // 1) decode to float stereo
    const std::size_t bytesPerSample = static_cast<std::size_t>(bits / 8);
    const std::size_t srcFrames = dataSize / (bytesPerSample * static_cast<std::size_t>(channels));
    std::vector<float> src(srcFrames * 2);
    for (std::size_t i = 0; i < srcFrames; ++i) {
        float s[2];
        for (int c = 0; c < channels; ++c) {
            const std::uint8_t* p = data + (i * static_cast<std::size_t>(channels) + static_cast<std::size_t>(c)) * bytesPerSample;
            s[c] = bits == 16 ? static_cast<std::int16_t>(ReadU16(p)) / 32768.0f
                              : (static_cast<int>(p[0]) - 128) / 128.0f;
        }
        src[i * 2] = s[0];
        src[i * 2 + 1] = channels == 2 ? s[1] : s[0];
    }

    // 2) resample to the device rate with linear interpolation
    auto sound = std::make_shared<Sound>();
    sound->m_sampleRate = m_sampleRate;
    const std::size_t dstFrames = srcFrames == 0 ? 0 :
        static_cast<std::size_t>(static_cast<double>(srcFrames) * m_sampleRate / rate);
    sound->m_samples.resize(dstFrames * 2);
    const double step = static_cast<double>(rate) / m_sampleRate;
    for (std::size_t i = 0; i < dstFrames; ++i) {
        const double pos = static_cast<double>(i) * step;
        const std::size_t i0 = std::min(static_cast<std::size_t>(pos), srcFrames - 1);
        const std::size_t i1 = std::min(i0 + 1, srcFrames - 1);
        const float t = static_cast<float>(pos - static_cast<double>(i0));
        for (int c = 0; c < 2; ++c) {
            const float v = src[i0 * 2 + static_cast<std::size_t>(c)] * (1.0f - t) +
                            src[i1 * 2 + static_cast<std::size_t>(c)] * t;
            sound->m_samples[i * 2 + static_cast<std::size_t>(c)] =
                static_cast<std::int16_t>(std::clamp(v, -1.0f, 1.0f) * 32767.0f);
        }
    }
    return sound;
}

int AudioSystem::FindVoice(VoiceId id) const {
    if (id < 0) return -1;
    const int index = id % VoiceCount;
    const auto generation = static_cast<std::uint32_t>(id / VoiceCount);
    return (m_voices[index].active && m_voices[index].generation == generation) ? index : -1;
}

AudioSystem::VoiceId AudioSystem::Play(std::shared_ptr<const Sound> sound, float volume, bool loop) {
    if (m_device == 0 || !sound || sound->GetFrameCount() == 0) {
        return InvalidVoice;
    }
    SDL_LockAudioDevice(m_device);
    VoiceId result = InvalidVoice;
    for (int i = 0; i < VoiceCount; ++i) {
        Voice& v = m_voices[i];
        if (!v.active) {
            v.sound = std::move(sound);
            v.frame = 0;
            v.volume = std::clamp(volume, 0.0f, 1.0f);
            v.loop = loop;
            v.active = true;
            ++v.generation;
            result = static_cast<VoiceId>(v.generation) * VoiceCount + i;
            break;
        }
    }
    SDL_UnlockAudioDevice(m_device);
    return result;
}

void AudioSystem::SetVoiceVolume(VoiceId voice, float volume) {
    if (m_device == 0) return;
    SDL_LockAudioDevice(m_device);
    const int index = FindVoice(voice);
    if (index >= 0) m_voices[index].volume = std::clamp(volume, 0.0f, 1.0f);
    SDL_UnlockAudioDevice(m_device);
}

void AudioSystem::Stop(VoiceId voice) {
    if (m_device == 0) return;
    SDL_LockAudioDevice(m_device);
    const int index = FindVoice(voice);
    if (index >= 0) {
        m_voices[index].active = false;
        m_voices[index].sound.reset();
    }
    SDL_UnlockAudioDevice(m_device);
}

void AudioSystem::StopAll() {
    if (m_device == 0) return;
    SDL_LockAudioDevice(m_device);
    for (Voice& v : m_voices) {
        v.active = false;
        v.sound.reset();
    }
    SDL_UnlockAudioDevice(m_device);
}

bool AudioSystem::IsPlaying(VoiceId voice) const {
    if (m_device == 0) return false;
    SDL_LockAudioDevice(m_device);
    const bool playing = FindVoice(voice) >= 0;
    SDL_UnlockAudioDevice(m_device);
    return playing;
}

void AudioSystem::SetMasterVolume(float volume) {
    if (m_device == 0) return;
    SDL_LockAudioDevice(m_device);
    m_masterVolume = std::clamp(volume, 0.0f, 1.0f);
    SDL_UnlockAudioDevice(m_device);
}

void AudioSystem::MixInto(std::int16_t* out, int frames) {
    // SDL already holds the device lock while it calls us.
    std::fill(out, out + static_cast<std::size_t>(frames) * 2, static_cast<std::int16_t>(0));
    static thread_local std::vector<float> mix;
    mix.assign(static_cast<std::size_t>(frames) * 2, 0.0f);

    for (Voice& v : m_voices) {
        if (!v.active) continue;
        const std::int16_t* samples = v.sound->m_samples.data();
        const std::size_t total = static_cast<std::size_t>(v.sound->GetFrameCount());
        const float gain = v.volume * m_masterVolume * (1.0f / 32768.0f);
        for (int i = 0; i < frames; ++i) {
            if (v.frame >= total) {
                if (!v.loop) {
                    v.active = false; // the Sound is released when the voice slot is reused or stopped
                    break;
                }
                v.frame = 0;
            }
            mix[static_cast<std::size_t>(i) * 2] += samples[v.frame * 2] * gain;
            mix[static_cast<std::size_t>(i) * 2 + 1] += samples[v.frame * 2 + 1] * gain;
            ++v.frame;
        }
    }
    for (std::size_t i = 0; i < mix.size(); ++i) {
        out[i] = static_cast<std::int16_t>(std::clamp(mix[i], -1.0f, 1.0f) * 32767.0f);
    }
}

} // namespace engine
