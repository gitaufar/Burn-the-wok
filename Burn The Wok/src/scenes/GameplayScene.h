#pragma once

#include "audio/SoundBank.h"
#include "entities/Player.h"
#include "systems/FireSystem.h"
#include "systems/Sprayer.h"
#include "systems/StageRules.h"
#include "world/ForestMap.h"

#include <engine/core/Scene.h>
#include <engine/entity/EntityWorld.h>
#include <engine/graphics/Camera2D.h>
#include <engine/graphics/Texture.h>

#include <memory>
#include <vector>

namespace game {

// One campaign stage. The ranger puts out fires, refills at the pond and guides
// wildlife to shelter. The current stage determines which antagonists appear.
// WASD/arrows = move, hold left mouse = spray toward the cursor, mouse wheel = zoom,
// F1/F2 = jump to win/lose screen (debug), F3 = show hitboxes and solid tiles (debug),
// F4 = set the tree under the cursor on fire (debug),
// Hold Space = calm nearby wildlife, Q = call campaign volunteers, Esc = pause.
class GameplayScene : public engine::Scene {
public:
    explicit GameplayScene(config::StageId stage = config::StageId::FirstFire);

    void OnEnter() override;
    void OnExit() override;
    void Update(float dt) override;
    void Draw(float alpha) override;

private:
    void UpdateCamera(float alpha);
    void DrawOverlay(float alpha);
    void DrawCollisionDebug(float alpha); // inside the world batch (camera space)
    void DrawFires(float alpha, float playerFeetY, bool foreground);
    // Water bursting out of the spray gun's nozzle (the gun itself is part of the ranger sprite).
    void DrawWaterBurst(float alpha);
    void RebuildMinimap();
    void UpdatePause();
    void DrawPause();
    void SpawnActors();
    void SpawnVolunteers(int count, float duty);
    void DrawRescueHints(float alpha);
    // Sound effects and music, driven by what happened this update.
    void UpdateSound(float dt, bool trigger, int firesPutOut, const FireSystem::StepResult& step,
                     StageOutcome outcome);
    void StopSounds();
    void FinishStage(); // switch to the win/lose screen

    std::unique_ptr<engine::Texture> m_atlas; // created in OnEnter (needs the GL context)
    std::unique_ptr<engine::Texture> m_treeTexture;
    std::unique_ptr<engine::Texture> m_minimap;
    // Image assets from assets/items, with the visible (non-transparent) part of each.
    struct Sprite {
        std::unique_ptr<engine::Texture> texture;
        engine::Rect source;
    };
    static Sprite LoadSprite(const char* path);
    Sprite m_fireSprite;
    Sprite m_sawitSprite;
    Sprite m_waterSprayLeft, m_waterSprayRight;
    int m_minimapRevision = -1; // map revision the minimap was drawn from
    std::unique_ptr<engine::Texture> m_vignette;
    engine::Rect m_treeSource;
    config::StageId m_stage;
    const config::StageConfig& m_stageConfig;
    ForestMap m_map;
    Player m_player;
    engine::EntityWorld m_entities;            // fires, water drops, smoke
    std::unique_ptr<FireSystem> m_fireSystem; // created in OnEnter (needs the atlas)
    Sprayer m_sprayer;
    StageRules m_rules; // after m_map: counts the map's trees when constructed
    bool m_paused = false;
    int m_pauseSelection = 0;
    int m_firesPutOut = 0;
    int m_rescued = 0;
    int m_wildlifePoints = 0;
    int m_campaignTier = 0;
    float m_campaignTime = 0;
    float m_duty = 0;
    float m_dutyTotal = 0;
    bool m_called = false;
    bool m_reportQueued = false;
    struct RescuePopup { engine::Vec2 position; int points; float age; };
    std::vector<RescuePopup> m_popups;
    int m_pendingAnimals = 0;
    bool m_rangerBlocked = false;
    engine::Vec2 m_aim{1.0f, 0.0f}; // last spray direction (for drawing)
    float m_endTimer = 0.0f; // seconds since the outcome was decided
    engine::Camera2D m_camera;
    int m_zoom;
    float m_time = 0.0f;
    bool m_showCollision = false; // F3

    // Sound (levels in config::sound). Looping voices are started and stopped as the situation changes.
    std::unique_ptr<SoundBank> m_sfx;
    std::shared_ptr<const engine::Sound> m_music;
    engine::AudioSystem::VoiceId m_musicVoice = engine::AudioSystem::InvalidVoice;
    engine::AudioSystem::VoiceId m_sprayVoice = engine::AudioSystem::InvalidVoice;
    engine::AudioSystem::VoiceId m_fireVoice = engine::AudioSystem::InvalidVoice;
    engine::AudioSystem::VoiceId m_tickVoice = engine::AudioSystem::InvalidVoice;
    float m_musicLevel = 0.0f; // 0..1 fade of the music
    float m_stepTimer = 0.0f;
    float m_refillTimer = 0.0f;
    float m_emptyTimer = 0.0f;
};

} // namespace game
