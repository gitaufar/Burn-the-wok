#include "scenes/GameplayScene.h"

#include "GameConfig.h"
#include "entities/StageActor.h"
#include "entities/Volunteer.h"
#include "progress/SaveData.h"
#include "art/FieldIcons.h"
#include "art/PlaceholderArt.h"
#include "art/PixelText.h"
#include "entities/Fire.h"
#include "entities/Particle.h"
#include "entities/WaterDrop.h"
#include "scenes/LoadingScene.h"
#include "scenes/MenuScene.h"
#include "scenes/ResultScene.h"

#include <engine/core/Application.h>
#include <engine/core/Log.h>
#include <engine/graphics/Image.h>
#include <engine/physics/Collision.h>

#include <algorithm>
#include <cmath>
#include <exception>
#include <optional>
#include <string>
#include <utility>

namespace game {

using engine::Color;
using engine::Key;
using engine::Vec2;
using navigation::Cell;
using navigation::ToCell;
using navigation::CellCenter;
using navigation::FindRoute;

namespace {
// The result screen, behind a short loading screen.
std::unique_ptr<engine::Scene> LoadReport(ResultScene::Outcome outcome, ResultScene::Summary summary = {}) {
    return std::make_unique<LoadingScene>(
        [outcome, summary] { return std::make_unique<ResultScene>(outcome, summary); },
        "MENYUSUN LAPORAN PATROLI");
}
// The forest has its own viewport. HUD panels never cover playable terrain.
StageActor* RescueTarget(engine::EntityWorld& world, Vec2 ranger) {
    StageActor* closest = nullptr;
    float distance = StageActor::CalmRange * StageActor::CalmRange;
    world.ForEach<StageActor>([&](StageActor& actor) {
        if (!actor.CanCalm()) return;
        const float range = (actor.GetPosition()-ranger).LengthSquared();
        if (range <= distance) { distance = range; closest = &actor; }
    });
    return closest;
}
struct PatrolLayout {
    float scale;
    engine::Rect arena;
    explicit PatrolLayout(Vec2 size) {
        const float fit = std::min(size.x / 640.0f, size.y / 360.0f);
        scale = fit >= 1 ? std::floor(fit) : std::max(0.01f, fit);
        const float top = std::round(26 * scale);
        const float bottom = std::round(64 * scale);
        arena = {0, top, size.x, std::max(1.0f, size.y - top - bottom)};
    }
};
}

GameplayScene::GameplayScene(config::StageId stage)
    : m_stage(stage), m_stageConfig(config::GetStage(stage)),
      m_map(config::MapWidthTiles, config::MapHeightTiles, config::MapSeed),
      m_player(m_map.GetCenter(), TankSizes[SaveData::Current().upgrades[Tank]],
               1.0f + 0.40f * SaveData::Current().upgrades[Pump]),
      m_sprayer(config::FireSeed + 1, config::DropPower * (1.0f + 0.15f * SaveData::Current().upgrades[Nozzle])),
      m_rules(m_stageConfig.duration, m_stageConfig.sawitLimit, m_map.CountTiles(TileType::Tree),
              m_stageConfig.rescueTarget),
      m_pendingAnimals(m_stageConfig.animalCount),
      m_zoom(config::DefaultZoom) {}

void GameplayScene::OnEnter() {
    m_campaignTier = SaveData::Current().campaignTier;
    m_campaignTime = SaveData::Current().campaignSeconds;
    m_player.LoadArtwork();
    m_atlas = std::make_unique<engine::Texture>(BuildPlaceholderAtlas());
    m_fireSprite = LoadSprite("assets/items/fire.png");
    m_sawitSprite = LoadSprite("assets/items/palm_oil_tree.png");
    m_waterSprayLeft = LoadSprite("assets/items/water_spray_left.png");
    m_waterSprayRight = LoadSprite("assets/items/water_spray_right.png");
    m_entities.Clear();
    m_fireSystem = std::make_unique<FireSystem>(
        FireArt{m_atlas.get(), m_fireSprite.texture.get(), m_fireSprite.source}, config::FireSeed,
        m_stageConfig.spreadInterval, m_stage == config::StageId::Sabotage);
    m_fireSystem->IgniteRandomTrees(m_entities, m_map, m_stageConfig.initialFires);
    const engine::Image tree = engine::Image::LoadFromFile("assets/environment/Tree.png");
    m_treeSource = tree.GetContentBounds(16);
    m_treeTexture = std::make_unique<engine::Texture>(tree);
    SpawnActors();
    SpawnVolunteers(SaveData::Current().upgrades[Crew], 0);
    RebuildMinimap();

    engine::Image vignette(128, 72);
    for (int y = 0; y < 72; ++y) {
        for (int x = 0; x < 128; ++x) {
            const float dx = std::abs((x - 63.5f) / 63.5f);
            const float dy = std::abs((y - 35.5f) / 35.5f);
            const float edge = std::max(dx, dy);
            vignette.SetPixel(x, y, Color::FromHex(0x102C22,
                static_cast<std::uint8_t>(edge * edge * edge * 68.0f)));
        }
    }
    m_vignette = std::make_unique<engine::Texture>(vignette);

    m_sfx = std::make_unique<SoundBank>(App().GetAudio());
    m_sfx->Load({"stage_start", "footstep", "footstep_grass", "water_spray", "water_refill", "tank_empty",
                 "fire_loop", "fire_out", "fire_spread", "tree_to_palm", "timer_warning",
                 "rescue_calm", "animal_panic_orangutan", "animal_panic_bekantan",
                 "animal_panic_burung_hantu", "animal_saved", "animal_burned",
                 "volunteer_join", "volunteer_call", "volunteer_leave", "pause_in", "pause_out",
                 "saboteur_ignite", "saboteur_plant_seed", "boss_appear"});
    try {
        m_music = App().GetAudio().LoadSound("assets/audio/music/gameplay.wav");
        m_musicVoice = App().GetAudio().Play(m_music, 0.0f, true); // faded in from UpdateSound()
    } catch (const std::exception& e) {
        engine::LogError("Gameplay music unavailable: %s", e.what()); // the game still works in silence
    }
    m_sfx->Play("stage_start", config::sound::StageStart);
    if (SaveData::Current().upgrades[Crew]) m_sfx->Play("volunteer_join", 0.45f);
    if (m_stage == config::StageId::LastForest) m_sfx->Play("boss_appear", 0.7f);

    App().GetWindow().SetTitle(std::string(config::WindowTitle) + " - " + m_stageConfig.name +
                               " | Spasi: rescue, Q: relawan, Esc: jeda");
    App().GetWindow().SetClearColor(Color::FromHex(0x1B3027));
}

void GameplayScene::SpawnActors() {
    constexpr StageActor::Cell AnimalOffsets[] = {{-7, -3}, {5, 5}, {6, -5}, {-6, 5}};
    const auto center = ToCell(m_map.GetCenter());
    const auto spawn = [&](config::ActorKind kind, StageActor::Cell offset) {
        // Find a reachable tile near the intended start and outside the shelter.
        const StageActor::Cell target{center.x + offset.x, center.y + offset.y};
        const auto route = FindRoute(m_map, center, [&](StageActor::Cell cell) {
            const int distance = std::abs(cell.x - target.x) + std::abs(cell.y - target.y);
            return distance <= 1;
        });
        const auto cell = route.empty() ? center : route.front();
        m_entities.Spawn<StageActor>(kind, CellCenter(cell)).LoadArtwork();
    };
    std::vector<Cell> usedLandings, usedTrees;
    constexpr Cell PerchNeighbors[] = {{0,-1}, {-1,0}, {1,0}, {0,1}};
    const auto distance = [](Cell a, Cell b) { return std::abs(a.x-b.x) + std::abs(a.y-b.y); };
    const auto occupied = [](const std::vector<Cell>& cells, Cell site) {
        return std::find(cells.begin(), cells.end(), site) != cells.end();
    };
    for (int i = 0; i < m_stageConfig.animalCount; ++i) {
        const auto kind = m_stageConfig.animals[i];
        const Cell target{center.x + AnimalOffsets[i].x, center.y + AnimalOffsets[i].y};
        Cell landing = center;
        int groundScore = m_map.GetWidth() + m_map.GetHeight() + 1;
        int perchScore = groundScore + 10;
        std::optional<Cell> perch;
        Cell perchLanding = center;

        // Visit reachable ground in route order. Keep a clear tile beside each occupied tree
        // for the ranger to approach and for the animal to land before running to shelter.
        FindRoute(m_map, center, [&](Cell cell) {
            if (occupied(usedLandings, cell)) return false;
            const auto tile = m_map.GetTile(cell.x, cell.y);
            const bool grass = tile == TileType::Grass || tile == TileType::GrassFlowers;
            const int score = distance(cell, target) + (grass ? 0 : 4);
            if (score < groundScore) { groundScore = score; landing = cell; }
            if (!m_stageConfig.perchAnimals[i]) return false;
            for (const Cell neighbor : PerchNeighbors) {
                const Cell tree{cell.x+neighbor.x, cell.y+neighbor.y};
                if (!m_map.IsBurnable(tree.x, tree.y) || occupied(usedTrees, tree)) continue;
                int score = distance(tree, target)*2 + (neighbor.y == -1 ? 0 : 2);
                // Prefer a crown that reads clearly above nearby foreground trees.
                for (int dx = -1; dx <= 1; ++dx)
                    if (m_map.GetTile(tree.x+dx, tree.y+1) == TileType::Tree) score += 2;
                if (score >= perchScore) continue;
                perchScore = score;
                perch = tree;
                perchLanding = cell;
            }
            return false;
        });
        if (perch) landing = perchLanding;
        usedLandings.push_back(landing);
        auto& animal = m_entities.Spawn<StageActor>(kind, CellCenter(landing));
        if (perch) {
            const auto tree = m_map.GetTreeSpriteRect(perch->x, perch->y, m_treeSource.w / m_treeSource.h);
            // Normalized contacts on Tree.png: the high fork and the two lower limbs.
            const Vec2 contact = kind == config::ActorKind::Owl ? Vec2{0.60f, 0.35f} :
                kind == config::ActorKind::Bekantan ? Vec2{0.35f, 0.59f} : Vec2{0.72f, 0.57f};
            animal.SetPerch(*perch, {tree.x + tree.w*contact.x, tree.y + tree.h*contact.y});
            usedTrees.push_back(*perch);
        }
        animal.LoadArtwork();
    }
    if (m_stage == config::StageId::Sabotage) spawn(config::ActorKind::Saboteur, {6, -1});
    if (m_stage == config::StageId::LastForest) {
        spawn(config::ActorKind::Wowo, {-4, 3});
        spawn(config::ActorKind::Tedduy, {4, -3});
    }
}

void GameplayScene::SpawnVolunteers(int count, float duty) {
    const auto center = ToCell(m_map.GetCenter());
    for (int i = 0; i < count; ++i) {
        const auto route = FindRoute(m_map, center, [&](Cell cell) {
            if (duty > 0) return i == 0 ? cell.x == 1 : i == 1 ? cell.x == m_map.GetWidth()-2 : cell.y == 1;
            return cell == Cell{center.x + i*2 - 1, center.y + 2};
        });
        if (!route.empty()) m_entities.Spawn<Volunteer>(CellCenter(route.front()), 71u + i, duty);
    }
}

void GameplayScene::OnExit() {
    auto& save = SaveData::Current();
    if (!m_reportQueued && m_campaignTime != save.campaignSeconds) {
        auto next = save;
        next.campaignSeconds = m_campaignTime;
        if (!save.Commit(next)) engine::LogError("Campaign time could not be saved.");
    }
    StopSounds();
    if (m_paused) App().GetAudio().SetPaused(false);
}

void GameplayScene::StopSounds() {
    engine::AudioSystem& audio = App().GetAudio();
    for (engine::AudioSystem::VoiceId* voice : {&m_musicVoice, &m_sprayVoice, &m_fireVoice, &m_tickVoice}) {
        audio.Stop(*voice);
        *voice = engine::AudioSystem::InvalidVoice;
    }
}

void GameplayScene::Update(float dt) {
    if (m_rules.GetOutcome() != StageOutcome::Playing) {
        m_endTimer += dt;
        UpdateSound(dt, false, 0, {}, m_rules.GetOutcome());
        if (m_endTimer >= config::ResultDelay) FinishStage();
        return;
    }
    const engine::Input& input = App().GetInput();
    if (input.IsKeyPressed(Key::Escape)) {
        m_paused = !m_paused;
        if (m_paused) m_pauseSelection = 0;
        App().GetAudio().SetPaused(m_paused);
        m_sfx->Play(m_paused ? "pause_in" : "pause_out", config::sound::UiClick);
        return;
    }
    if (m_paused) {
        UpdatePause();
        return;
    }
    m_time += dt;
    if (m_campaignTime > 0 && !m_called && input.IsKeyPressed(Key::Q)) {
        m_called = true;
        m_duty = m_dutyTotal = m_campaignTier == 2 ? 45.0f : 30.0f;
        SpawnVolunteers(m_campaignTier+1, m_duty);
        m_sfx->Play("volunteer_call", 0.65f);
    }
    m_campaignTime = std::max(0.0f, m_campaignTime - dt);
    if (m_duty > 0) {
        m_duty = std::max(0.0f, m_duty - dt);
        if (m_duty == 0) m_sfx->Play("volunteer_leave", 0.45f);
    }
    for (auto& popup : m_popups) popup.age += dt;
    std::erase_if(m_popups, [](const RescuePopup& popup) { return popup.age >= 1.2f; });
    // Debug shortcuts for checking the result screens.
    if (input.IsKeyPressed(Key::F1)) {
        App().GetScenes().SwitchTo(LoadReport(ResultScene::Outcome::Win));
        return;
    }
    if (input.IsKeyPressed(Key::F2)) {
        App().GetScenes().SwitchTo(LoadReport(ResultScene::Outcome::Lose));
        return;
    }

    if (const int wheel = input.GetMouseWheel(); wheel != 0) {
        m_zoom = std::clamp(m_zoom + wheel, config::MinZoom, config::MaxZoom);
    }

    // Debug: show hitboxes and solid tiles (useful when tuning collision).
    if (input.IsKeyPressed(Key::F3)) {
        m_showCollision = !m_showCollision;
    }

    m_rangerBlocked = false;
    std::as_const(m_entities).ForEach<StageActor>([&](const StageActor& actor) {
        if (actor.BlocksRanger() && (actor.GetPosition() - m_player.GetPosition()).LengthSquared() < config::BossBlockDistance * config::BossBlockDistance) {
            m_rangerBlocked = true;
        }
    });
    StageActor* calming = input.IsKeyDown(Key::Space) ? RescueTarget(m_entities, m_player.GetPosition()) : nullptr;
    const float boots = 1.0f + 0.1f * SaveData::Current().upgrades[Boots];
    m_player.Update(input, dt, m_map, calming ? 0.0f : boots * (m_rangerBlocked ? config::BossSlowMultiplier : 1.0f));

    // Spray toward the mouse. The camera is from the last drawn frame, which is what
    // the player is looking at when they click.
    const PatrolLayout layout(App().GetWindow().GetSize());
    const bool inArena = layout.arena.Contains(input.GetMousePosition());
    const Vec2 mouseWorld = m_camera.ScreenToWorld(input.GetMousePosition() - layout.arena.Position());
    const Vec2 aim = (mouseWorld - m_player.GetPosition()).Normalized();
    const bool trigger = inArena && input.IsMouseDown(engine::MouseButton::Left);
    if (trigger) {
        m_player.Aim(aim);
        if (aim != Vec2{}) m_aim = aim;
    }

    m_sprayer.Update(dt, trigger, m_player.GetNozzlePosition(aim), aim, m_player.GetTank(), m_entities);

    // Debug: light the tree under the cursor.
    if (inArena && input.IsKeyPressed(Key::F4)) {
        const float ts = static_cast<float>(config::TileSize);
        m_fireSystem->Ignite(m_entities, m_map, static_cast<int>(std::floor(mouseWorld.x / ts)),
                             static_cast<int>(std::floor(mouseWorld.y / ts)));
    }

    m_entities.Update(dt);
    m_entities.ForEach<Volunteer>([&](Volunteer& volunteer) { volunteer.Step(dt, m_map, m_entities); });
    const int putOut = m_fireSystem->ResolveWaterHits(m_entities);
    m_firesPutOut += putOut;
    FireSystem::StepResult step = m_fireSystem->Update(m_entities, m_map, dt);
    m_rescued = 0;
    m_wildlifePoints = 0;
    m_pendingAnimals = 0;
    m_entities.ForEach<StageActor>([&](StageActor& actor) {
        using Rescue = StageActor::Rescue;
        const auto before = actor.GetRescue();
        const int previousIgnitions = actor.GetIgnitionsLeft();
        actor.Step(dt, m_map, m_entities, *m_fireSystem, m_player.GetPosition(), m_player.GetHitbox(),
                   &actor == calming);
        const auto after = actor.GetRescue();
        if (after == Rescue::Calming && before != after) m_sfx->Play("rescue_calm", 0.35f);
        if (after == Rescue::Fleeing && before != after) {
            constexpr const char* Panic[] = {"animal_panic_orangutan", "animal_panic_bekantan", "animal_panic_burung_hantu"};
            m_sfx->Play(Panic[static_cast<int>(actor.GetKind())], 0.5f);
            m_popups.push_back({actor.GetVisualPosition(1.0f), RescuePoints(actor.GetKind()), 0});
        }
        if (after == Rescue::Saved && before != after) m_sfx->Play("animal_saved", 0.55f);
        if (after == Rescue::Burned && before != after) m_sfx->Play("animal_burned", 0.42f);
        if (actor.IsPlanting()) {
            ++step.turnedToSawit;
            m_sfx->Play("saboteur_plant_seed", 0.7f);
        }
        if (actor.GetIgnitionsLeft() < previousIgnitions) m_sfx->Play("saboteur_ignite", 0.7f);
        if (!actor.IsAnimal()) return;
        if (actor.GetRescue() == StageActor::Rescue::Saved) {
            ++m_rescued;
            m_wildlifePoints += RescuePoints(actor.GetKind());
        }
        else if (actor.GetRescue() != StageActor::Rescue::Burned) ++m_pendingAnimals;
    });
    if (m_map.GetRevision() != m_minimapRevision) {
        RebuildMinimap();
    }

    // The outcome is shown in the world for a moment before the result screen.
    const StageOutcome outcome =
        m_rules.Update(dt, m_entities.Count<Fire>(), m_map.CountTiles(TileType::Sawit),
                       m_map.CountTiles(TileType::Charred), m_rescued, m_pendingAnimals);
    UpdateSound(dt, trigger, putOut, step, outcome);
}

namespace {
engine::Rect PauseButton(engine::Vec2 size, int index) {
    const float scale = std::min(size.x / 640.0f, size.y / 360.0f);
    return {size.x * 0.5f - 135 * scale, size.y * 0.5f - 25 * scale + index * 36 * scale,
            270 * scale, 29 * scale};
}
}

void GameplayScene::UpdatePause() {
    const auto& input = App().GetInput();
    const auto size = App().GetWindow().GetSize();
    if (input.IsKeyPressed(Key::Up) || input.IsKeyPressed(Key::W)) m_pauseSelection = (m_pauseSelection + 2) % 3;
    if (input.IsKeyPressed(Key::Down) || input.IsKeyPressed(Key::S)) m_pauseSelection = (m_pauseSelection + 1) % 3;
    bool activate = input.IsKeyPressed(Key::Enter) || input.IsKeyPressed(Key::NumpadEnter) ||
                    input.IsKeyPressed(Key::Space);
    for (int i = 0; i < 3; ++i) {
        if (PauseButton(size, i).Contains(input.GetMousePosition()) &&
            input.IsMousePressed(engine::MouseButton::Left)) {
            m_pauseSelection = i;
            activate = true;
        }
    }
    if (!activate) return;
    if (m_pauseSelection == 0) {
        m_paused = false;
        App().GetAudio().SetPaused(false);
        m_sfx->Play("pause_out", config::sound::UiClick);
    } else if (m_pauseSelection == 1) {
        App().GetScenes().SwitchTo(LoadStage(m_stage));
    } else {
        App().GetScenes().SwitchTo(std::make_unique<LoadingScene>(
            [] { return std::make_unique<MenuScene>(); }, "KEMBALI KE POS JAGA"));
    }
}

void GameplayScene::DrawPause() {
    const auto size = App().GetWindow().GetSize();
    const float scale = std::min(size.x / 640.0f, size.y / 360.0f);
    engine::Camera2D camera;
    camera.SetViewportSize(size);
    camera.SetPosition(size * 0.5f);
    auto& batch = App().GetSpriteBatch();
    batch.Begin(camera.GetViewProjection());
    batch.DrawRect({0, 0, size.x, size.y}, Color::FromHex(0x081B15, 210));
    const auto first = PauseButton(size, 0);
    batch.DrawRect({first.x - 20*scale, first.y - 65*scale, 310*scale, 192*scale}, Color::FromHex(0xE8DCBB));
    art::DrawPixelText(batch, "PATROLI DIJEDA", first.x, first.y - 47*scale, 2*scale, Color::FromHex(0x283C32));
    art::DrawPixelText(batch, "WAKTU DAN SIMULASI BERHENTI", first.x, first.y - 23*scale, scale, Color::FromHex(0x73765F));
    constexpr const char* labels[] = {"LANJUTKAN", "ULANGI STAGE", "KEMBALI KE MENU"};
    for (int i = 0; i < 3; ++i) {
        const auto box = PauseButton(size, i);
        const bool selected = i == m_pauseSelection || box.Contains(App().GetInput().GetMousePosition());
        batch.DrawRect(box, Color::FromHex(selected ? 0x48664A : 0x283C32));
        art::DrawPixelText(batch, labels[i], box.x + 12*scale, box.y + 10*scale, scale, Color::FromHex(0xF8EDCE));
    }
    batch.End();
}

void GameplayScene::UpdateSound(float dt, bool trigger, int firesPutOut, const FireSystem::StepResult& step,
                                StageOutcome outcome) {
    namespace snd = config::sound;
    engine::AudioSystem& audio = App().GetAudio();
    const bool playing = outcome == StageOutcome::Playing;

    // Music fades in at the start and ducks once the stage is decided.
    const float musicTarget = playing ? 1.0f : 0.25f;
    m_musicLevel += (musicTarget - m_musicLevel) * std::min(1.0f, dt * (playing ? 0.8f : 2.0f));
    audio.SetVoiceVolume(m_musicVoice, snd::GameplayMusic * m_musicLevel);

    // Water stream: a loop that runs exactly while water comes out of the nozzle.
    const bool spraying = playing && m_sprayer.IsSpraying();
    if (spraying && m_sprayVoice == engine::AudioSystem::InvalidVoice) {
        m_sprayVoice = m_sfx->Loop("water_spray", snd::Spray);
    } else if (!spraying && m_sprayVoice != engine::AudioSystem::InvalidVoice) {
        audio.Stop(m_sprayVoice);
        m_sprayVoice = engine::AudioSystem::InvalidVoice;
    }

    // Trigger held but nothing comes out: the tank is empty.
    m_emptyTimer = std::max(0.0f, m_emptyTimer - dt);
    if (playing && trigger && !m_sprayer.IsSpraying() &&
        m_player.GetTank().GetAmount() < config::WaterPerDrop && m_emptyTimer <= 0.0f) {
        m_sfx->Play("tank_empty", snd::TankEmpty);
        m_emptyTimer = snd::TankEmptyInterval;
    }

    // Refilling at the pond: plays immediately, then repeats while the tank is filling.
    if (playing && m_player.IsRefilling()) {
        m_refillTimer -= dt;
        if (m_refillTimer <= 0.0f) {
            m_sfx->Play("water_refill", snd::Refill);
            m_refillTimer = snd::RefillInterval;
        }
    } else {
        m_refillTimer = 0.0f;
    }

    // Footsteps: harder on the dirt trail, soft on grass.
    if (playing && m_player.IsMoving()) {
        m_stepTimer -= dt;
        if (m_stepTimer <= 0.0f) {
            const engine::Rect feet = m_player.GetHitbox();
            const float ts = static_cast<float>(config::TileSize);
            const TileType ground = m_map.GetTile(static_cast<int>(std::floor((feet.x + feet.w * 0.5f) / ts)),
                                                  static_cast<int>(std::floor((feet.y + feet.h * 0.5f) / ts)));
            m_sfx->Play(ground == TileType::Dirt ? "footstep" : "footstep_grass", snd::Footstep);
            m_stepTimer = snd::StepInterval;
        }
    } else {
        m_stepTimer = 0.0f;
    }

    // Fire ambience: louder with more fires and when the nearest one is close.
    const int fires = m_entities.Count<Fire>();
    if (playing && fires > 0) {
        float nearest = snd::FireHearingRange;
        const Vec2 player = m_player.GetPosition();
        std::as_const(m_entities).ForEach<Fire>([&](const Fire& fire) {
            nearest = std::min(nearest, (fire.GetHitArea().center - player).Length());
        });
        const float closeness = 1.0f - nearest / snd::FireHearingRange;       // 0 far .. 1 right next to it
        const float crowd = std::min(1.0f, static_cast<float>(fires) / 8.0f); // 0 .. 1 for 8+ fires
        const float volume = snd::FireLoopMin + (snd::FireLoopMax - snd::FireLoopMin) * (0.7f * closeness + 0.3f * crowd);
        if (m_fireVoice == engine::AudioSystem::InvalidVoice) {
            m_fireVoice = m_sfx->Loop("fire_loop", volume);
        } else {
            audio.SetVoiceVolume(m_fireVoice, volume);
        }
    } else if (m_fireVoice != engine::AudioSystem::InvalidVoice) {
        audio.Stop(m_fireVoice);
        m_fireVoice = engine::AudioSystem::InvalidVoice;
    }

    // One-shot events of this update.
    if (firesPutOut > 0) m_sfx->Play("fire_out", snd::FireOut);
    if (step.spread > 0) m_sfx->Play("fire_spread", snd::FireSpread);
    if (step.turnedToSawit > 0) m_sfx->Play("tree_to_palm", snd::TreeToPalm);

    // Ticking in the last seconds.
    const bool urgent = playing && m_rules.GetTimeLeft() <= snd::TimerWarningSeconds;
    if (urgent && m_tickVoice == engine::AudioSystem::InvalidVoice) {
        m_tickVoice = m_sfx->Loop("timer_warning", snd::TimerWarning);
    } else if (!urgent && m_tickVoice != engine::AudioSystem::InvalidVoice) {
        audio.Stop(m_tickVoice);
        m_tickVoice = engine::AudioSystem::InvalidVoice;
    }
}

GameplayScene::Sprite GameplayScene::LoadSprite(const char* path) {
    const engine::Image image = engine::Image::LoadFromFile(path);
    // Trim the transparent canvas so sizes are based on what is actually drawn.
    return {std::make_unique<engine::Texture>(image), image.GetContentBounds(16)};
}

void GameplayScene::DrawWaterBurst(float alpha) {
    // Only while water actually comes out, and only when the ranger faces sideways:
    // the burst images point left or right, like the gun in those sprites. Facing up
    // or down, the drops alone show the stream.
    const Facing facing = m_player.GetFacing();
    if (!m_sprayer.IsSpraying() || (facing != Facing::Left && facing != Facing::Right)) return;

    const bool right = facing == Facing::Right;
    const Sprite& burst = right ? m_waterSprayRight : m_waterSprayLeft;
    const float bh = std::round(9.0f + std::sin(m_time * 30.0f));
    const float bw = std::round(bh * burst.source.w / burst.source.h);
    const Vec2 nozzle = m_player.GetNozzlePosition(m_aim) +
                        (m_player.GetRenderPosition(alpha) - m_player.GetPosition());
    // The narrow end of the burst starts at the nozzle.
    const float bx = right ? nozzle.x - 1.0f : nozzle.x + 1.0f - bw;
    App().GetSpriteBatch().Draw(*burst.texture, {std::round(bx), std::round(nozzle.y - bh * 0.5f), bw, bh},
                                burst.source, Color::White().WithAlpha(225));
}

void GameplayScene::FinishStage() {
    ResultScene::Summary summary;
    switch (m_rules.GetOutcome()) {
    case StageOutcome::Won: summary.reason = "OBJEKTIF TERCAPAI"; break;
    case StageOutcome::LostSawit: summary.reason = "LAHAN SAWIT MELEWATI BATAS"; break;
    case StageOutcome::LostTime: summary.reason = "WAKTU HABIS"; break;
    case StageOutcome::LostAnimals: summary.reason = "TARGET SATWA GAGAL"; break;
    case StageOutcome::Playing: break;
    }
    summary.hasStats = true;
    summary.stage = m_stage;
    summary.rescued = m_rescued;
    summary.totalAnimals = m_stageConfig.animalCount;
    summary.rescueTarget = m_stageConfig.rescueTarget;
    summary.wildlifePoints = m_wildlifePoints;
    summary.campaignTime = m_campaignTime;
    summary.forestPercent = static_cast<int>(std::round(m_rules.GetForestFraction() * 100.0f));
    summary.sawitPercent = static_cast<int>(std::round(m_rules.GetSawitFraction() * 100.0f));
    summary.firesPutOut = m_firesPutOut;
    summary.secondsLeft = static_cast<int>(std::ceil(m_rules.GetTimeLeft()));
    const auto result = m_rules.GetOutcome() == StageOutcome::Won ? ResultScene::Outcome::Win
                                                                  : ResultScene::Outcome::Lose;
    m_reportQueued = true;
    App().GetScenes().SwitchTo(LoadReport(result, std::move(summary)));
}

void GameplayScene::RebuildMinimap() {
    engine::Image minimap(m_map.GetWidth(), m_map.GetHeight());
    for (int y = 0; y < m_map.GetHeight(); ++y) {
        for (int x = 0; x < m_map.GetWidth(); ++x) {
            Color color = Color::FromHex(0x708457);
            switch (m_map.GetTile(x, y)) {
            case TileType::Tree: color = Color::FromHex(0x36573B); break;
            case TileType::Water: color = Color::FromHex(0x5AADB0); break;
            case TileType::Dirt: color = Color::FromHex(0xC8AF7D); break;
            case TileType::Charred: color = Color::FromHex(0x52463E); break;
            case TileType::Sawit: color = Color::FromHex(0xC9A23A); break;
            default: break;
            }
            minimap.SetPixel(x, y, color);
        }
    }
    m_minimap = std::make_unique<engine::Texture>(minimap);
    m_minimapRevision = m_map.GetRevision();
}

void GameplayScene::DrawFires(float alpha, float playerFeetY, bool foreground) {
    // Same depth rule as the trees they burn on: roots below the player's feet = in front.
    std::as_const(m_entities).ForEach<Fire>([&](const Fire& fire) {
        if ((fire.GetBaseY() > playerFeetY) == foreground) {
            fire.Draw(App().GetSpriteBatch(), alpha);
        }
    });
}

void GameplayScene::DrawCollisionDebug(float alpha) {
    engine::SpriteBatch& batch = App().GetSpriteBatch();

    // Solid tiles near the player only, so the overlay stays readable.
    const Vec2 player = m_player.GetRenderPosition(alpha);
    const float ts = static_cast<float>(config::TileSize);
    const engine::Rect nearby{player.x - 4 * ts, player.y - 4 * ts, 8 * ts, 8 * ts};
    const engine::TileRange tiles = engine::TilesOverlapping(nearby, ts);
    for (int y = tiles.minY; y <= tiles.maxY; ++y) {
        for (int x = tiles.minX; x <= tiles.maxX; ++x) {
            if (m_map.IsSolid(x, y)) {
                batch.DrawRectOutline(m_map.GetTileRect(x, y), 0.5f, Color::FromHex(0xE0503C, 200));
            }
        }
    }

    // Draw the hitbox where the sprite is drawn (interpolated), not at the raw position.
    engine::Rect hitbox = m_player.GetHitbox();
    hitbox.x += player.x - m_player.GetPosition().x;
    hitbox.y += player.y - m_player.GetPosition().y;
    batch.DrawRect(hitbox, Color::FromHex(0x4FD1E8, 90));
    batch.DrawRectOutline(hitbox, 0.5f, Color::FromHex(0x4FD1E8, 230));
}

void GameplayScene::UpdateCamera(float alpha) {
    const Vec2 viewport = PatrolLayout(App().GetWindow().GetSize()).arena.Size();
    const float zoom = static_cast<float>(m_zoom);
    m_camera.SetViewportSize(viewport);
    m_camera.SetZoom(zoom);

    // Follow the player, but never show the void outside the map. If the map is
    // smaller than the screen on an axis, center it on that axis instead.
    const engine::Rect bounds = m_map.GetBounds();
    const Vec2 halfView = viewport / (2.0f * zoom);
    Vec2 target = m_player.GetRenderPosition(alpha);

    if (bounds.w > halfView.x * 2.0f) {
        target.x = std::clamp(target.x, bounds.Left() + halfView.x, bounds.Right() - halfView.x);
    } else {
        target.x = bounds.Center().x;
    }
    if (bounds.h > halfView.y * 2.0f) {
        target.y = std::clamp(target.y, bounds.Top() + halfView.y, bounds.Bottom() - halfView.y);
    } else {
        target.y = bounds.Center().y;
    }

    m_camera.SetPosition(target);
}

void GameplayScene::Draw(float alpha) {
    if (m_paused || m_rules.GetOutcome() != StageOutcome::Playing) alpha = 1.0f;
    UpdateCamera(alpha);

    engine::SpriteBatch& batch = App().GetSpriteBatch();
    const PatrolLayout layout(App().GetWindow().GetSize());
    App().GetWindow().SetViewport(layout.arena);
    batch.Begin(m_camera.GetViewProjection());

    const auto visibleArea = m_camera.GetVisibleArea();
    const float playerFeetY = m_player.GetRenderFeetY(alpha);
    m_map.DrawGround(batch, *m_atlas, visibleArea, m_time);
    m_map.DrawTrees(batch, *m_treeTexture, m_treeSource, *m_sawitSprite.texture, m_sawitSprite.source,
                    visibleArea, playerFeetY, false);
    DrawFires(alpha, playerFeetY, false);
    std::as_const(m_entities).ForEach<StageActor>([&](const StageActor& actor) {
        if (!actor.IsAboveGround()) actor.Draw(batch, alpha, static_cast<float>(m_zoom));
    });
    std::as_const(m_entities).ForEach<Volunteer>([&](const Volunteer& volunteer) { volunteer.Draw(batch, alpha); });
    const Vec2 shelter = CellCenter(ToCell(m_map.GetCenter()));
    batch.DrawRectOutline({shelter.x - 7, shelter.y - 7, 14, 14}, 1, Color::FromHex(0xB6D98A, 200));
    art::DrawFieldIcon(batch, art::FieldIcon::Flag, shelter.x-4, shelter.y-19, 1, Color::FromHex(0xDCE6AE));
    m_player.Draw(batch, *m_atlas, alpha, static_cast<float>(m_zoom));
    DrawWaterBurst(alpha);
    m_map.DrawTrees(batch, *m_treeTexture, m_treeSource, *m_sawitSprite.texture, m_sawitSprite.source,
                    visibleArea, playerFeetY, true);
    std::as_const(m_entities).ForEach<StageActor>([&](const StageActor& actor) {
        if (actor.IsAboveGround()) actor.Draw(batch, alpha, static_cast<float>(m_zoom));
    });
    DrawFires(alpha, playerFeetY, true);

    // The spray arcs over everything; smoke and steam rise above the canopy.
    const engine::EntityWorld& entities = m_entities;
    entities.ForEach<WaterDrop>([&](const WaterDrop& drop) { drop.Draw(batch, alpha); });
    entities.ForEach<Particle>([&](const Particle& puff) { puff.Draw(batch, alpha); });

    const engine::Rect bounds = m_map.GetBounds();
    for (int i = 0; i < 48; ++i) {
        const float x = std::fmod(i * 137.7f + m_time * 2.0f, bounds.w);
        const float y = std::fmod(i * 73.3f + m_time * 0.7f, bounds.h) +
                        std::sin(m_time * 0.8f + i) * 2.0f;
        batch.Draw(*m_atlas, {x, y, 16, 16}, GetSpriteSource(SpriteId::Leaf));
    }

    // Highlight the tile under the mouse: shows the screen -> world conversion working.
    const Vec2 mouseWorld = m_camera.ScreenToWorld(App().GetInput().GetMousePosition() - layout.arena.Position());
    const float ts = static_cast<float>(config::TileSize);
    const engine::Rect hovered{std::floor(mouseWorld.x / ts) * ts, std::floor(mouseWorld.y / ts) * ts,
                               ts, ts};
    if (layout.arena.Contains(App().GetInput().GetMousePosition()) && m_map.GetBounds().Contains(mouseWorld)) {
        const Color marker = Color::FromHex(0xF2DEA0, 130);
        batch.DrawRect({hovered.x, hovered.y, 4, 1}, marker);
        batch.DrawRect({hovered.x, hovered.y, 1, 4}, marker);
        batch.DrawRect({hovered.Right() - 4, hovered.Bottom() - 1, 4, 1}, marker);
        batch.DrawRect({hovered.Right() - 1, hovered.Bottom() - 4, 1, 4}, marker);
    }

    if (m_showCollision) {
        DrawCollisionDebug(alpha);
    }

    batch.DrawRect({shelter.x-20, shelter.y-30, 40, 8}, Color::FromHex(0x203327, 225));
    art::DrawPixelText(batch, "POS AMAN", shelter.x-16, shelter.y-28, 0.7f, Color::FromHex(0xEBF3C7));
    DrawRescueHints(alpha);
    batch.End();
    DrawOverlay(alpha);
    if (m_paused) DrawPause();
}

void GameplayScene::DrawRescueHints(float alpha) {
    auto& batch = App().GetSpriteBatch();
    const auto* target = RescueTarget(m_entities, m_player.GetPosition());
    std::as_const(m_entities).ForEach<StageActor>([&](const StageActor& actor) {
        if (!actor.CanCalm()) return;
        const Vec2 pos = actor.GetVisualPosition(alpha);
        if (actor.GetRescue() == StageActor::Rescue::Trapped) {
            const float x = &actor == target ? pos.x+20 : pos.x-4;
            batch.DrawRect({x, pos.y-36, 8, 10}, Color::FromHex(0x9E382C));
            art::DrawPixelText(batch, "!", x+2, pos.y-35, 0.9f, Color::FromHex(0xFFF0CB));
        }
        if (&actor != target) return;
        batch.DrawRect({pos.x-19, pos.y-36, 38, 10}, Color::FromHex(0x203327));
        art::DrawPixelText(batch, "[SPASI]", pos.x-16, pos.y-34, 0.8f, Color::FromHex(0xFFF0CB));
        if (actor.GetRescue() != StageActor::Rescue::Calming) return;
        for (int i = 0; i < 24; ++i) {
            const float angle = i * 6.2831853f / 24 - 1.5707963f;
            const bool filled = i < actor.CalmProgress() * 24;
            batch.DrawRect({pos.x+13*std::cos(angle)-1, pos.y-9+13*std::sin(angle)-1, 2, 2},
                           Color::FromHex(filled ? 0xDDE89F : 0x53664C));
        }
    });
    for (const auto& popup : m_popups) {
        const std::string label = "+" + std::to_string(popup.points);
        const float x = popup.position.x - 11;
        const float y = popup.position.y - 28 - popup.age*16;
        art::DrawPixelText(batch, label, x+1, y+1, 1.2f, Color::FromHex(0x233024));
        art::DrawPixelText(batch, label, x, y, 1.2f, Color::FromHex(0xF6D578));
    }
}

void GameplayScene::DrawOverlay(float alpha) {
    auto& batch = App().GetSpriteBatch();
    const Vec2 size = App().GetWindow().GetSize();
    const PatrolLayout layout(size);
    const float s = layout.scale, w = size.x / s, h = size.y / s;
    App().GetWindow().SetViewport({0, 0, size.x, size.y});
    engine::Camera2D camera;
    camera.SetViewportSize(size);
    camera.SetPosition(size * 0.5f);
    batch.Begin(camera.GetViewProjection());
    const Color ink = Color::FromHex(0x152A23), paper = Color::FromHex(0xE8D9AF);
    const Color cream = Color::FromHex(0xFFF0CB), muted = Color::FromHex(0xA9B69B);
    const Color moss = Color::FromHex(0x8FA66E), rule = Color::FromHex(0x415244);
    auto rect = [&](float x, float y, float width, float height, Color color) {
        batch.DrawRect({std::round(x*s), std::round(y*s), std::round(width*s), std::round(height*s)}, color);
    };
    auto text = [&](std::string_view value, float x, float y, Color color) {
        art::DrawPixelText(batch, value, std::round(x*s), std::round(y*s), std::max(1.0f, s), color);
    };
    // A thin location strip above the arena. No floating panels over the forest.
    rect(0, 0, w, 26, ink);
    rect(0, 25, w, 1, rule);
    rect(10, 7, 11, 11, moss);
    rect(14, 9, 3, 7, cream); rect(12, 11, 7, 2, cream);
    text("POS PENJAGA", 28, 9, cream);
    rect(103, 7, 1, 12, rule);
    text(m_stageConfig.name, 114, 9, muted);
    const int seconds = static_cast<int>(std::ceil(m_rules.GetTimeLeft()));
    const std::string clock = std::to_string(seconds / 60) + ":" +
        (seconds % 60 < 10 ? "0" : "") + std::to_string(seconds % 60);
    const int forest = static_cast<int>(std::round(m_rules.GetForestFraction() * 100));
    const int sawit = static_cast<int>(std::round(m_rules.GetSawitFraction() * 100));
    const int limit = static_cast<int>(std::round(m_rules.GetSawitLoseFraction() * 100));
    text("HUTAN " + std::to_string(forest) + "%", w - 300, 9, moss);
    text("SAWIT " + std::to_string(sawit) + "/" + std::to_string(limit) + "%", w - 207, 9, muted);
    rect(w - 103, 5, 1, 15, rule);
    text("WAKTU", w - 91, 9, muted);
    text(clock, w - 48, 9, seconds <= 30 ? Color::FromHex(0xF29B70) : cream);

    // The instrument rail is outside the world viewport, including the minimap.
    const float y = h - 64;
    rect(0, y, w, 64, ink);
    rect(0, y, w, 1, Color::FromHex(0x8C9C70));
    rect(0, y + 1, w, 2, Color::FromHex(0x0C1B16));
    const engine::Rect mapRect{10*s, (y+9)*s, 72*s, 48*s};
    rect(8, y+7, 76, 52, paper);
    batch.Draw(*m_minimap, mapRect);
    const auto world = m_map.GetBounds(), view = m_camera.GetVisibleArea();
    const float left = std::clamp(view.Left(), 0.0f, world.w);
    const float right = std::clamp(view.Right(), 0.0f, world.w);
    const float top = std::clamp(view.Top(), 0.0f, world.h);
    const float bottom = std::clamp(view.Bottom(), 0.0f, world.h);
    batch.DrawRectOutline({std::round(mapRect.x + left/world.w*mapRect.w),
        std::round(mapRect.y + top/world.h*mapRect.h),
        std::round((right-left)/world.w*mapRect.w), std::round((bottom-top)/world.h*mapRect.h)},
        s, Color::FromHex(0xFFF0CB, 160));
    auto mark = [&](Vec2 position, Color color, float radius) {
        // Keep markers inside the chart frame, including at the map edges.
        const float x = std::clamp(10 + position.x/world.w*72, 10 + radius, 82 - radius);
        const float my = std::clamp(y+9 + position.y/world.h*48, y+9 + radius, y+57 - radius);
        rect(x-radius, my-radius, radius*2+1, radius*2+1, color);
    };
    const bool blink = std::fmod(m_time, 0.6f) < 0.4f;
    std::as_const(m_entities).ForEach<Fire>([&](const Fire& fire) {
        mark(fire.GetPosition() - Vec2{0,8}, Color::FromHex(blink ? 0xFF703D : 0xFFC14D), 1);
    });
    const Vec2 shelter = CellCenter(ToCell(m_map.GetCenter()));
    art::DrawFieldIcon(batch, art::FieldIcon::Flag,
                       mapRect.x + shelter.x/world.w*mapRect.w - 2*s,
                       mapRect.y + shelter.y/world.h*mapRect.h - 4*s, 0.65f*s, cream);
    std::as_const(m_entities).ForEach<Volunteer>([&](const Volunteer& volunteer) {
        mark(volunteer.GetRenderPosition(alpha), Color::FromHex(0xE0AD54), 1);
    });
    std::as_const(m_entities).ForEach<StageActor>([&](const StageActor& actor) {
        if (actor.IsAnimal() && actor.GetRescue() != StageActor::Rescue::Saved && actor.GetRescue() != StageActor::Rescue::Burned)
            mark(actor.GetMapPosition(alpha), cream, 1);
    });
    mark(m_player.GetRenderPosition(alpha), Color::FromHex(0x101F1A), 2);
    mark(m_player.GetRenderPosition(alpha), Color::FromHex(0x91E4F1), 1);
    text("U", 94, y+9, paper);
    rect(96, y+20, 1, 8, muted); rect(94, y+21, 5, 1, muted);
    text(std::to_string(m_zoom)+"X", 90, y+42, muted);
    rect(113, y+10, 1, 44, rule);

    const auto& tank = m_player.GetTank();
    const int percent = static_cast<int>(std::round(tank.GetFraction()*100));
    const Color water = tank.GetFraction() < 0.2f ? Color::FromHex(0xF29B70) : Color::FromHex(0x73CBDF);
    text("AIR", 128, y+11, muted);
    rect(155, y+10, 115, 9, Color::FromHex(0x091A15));
    rect(157, y+12, std::round(111*tank.GetFraction()), 5, water);
    text(std::to_string(percent)+"%", 280, y+11, water);
    text("API " + std::to_string(m_entities.Count<Fire>()), 128, y+30, Color::FromHex(0xF4B376));
    text("PADAM " + std::to_string(m_firesPutOut), 207, y+30, muted);
    text("SATWA " + std::to_string(m_rescued) + "/" + std::to_string(m_stageConfig.rescueTarget), 128, y+47, moss);
    text("TARGET SELAMAT", 207, y+47, muted);
    rect(326, y+10, 1, 44, rule);

    std::string status = "TAHAN SPASI DEKAT SATWA";
    Color statusColor = paper;
    if (m_player.IsRefilling()) { status = "ISI AIR DI TEPI KOLAM"; statusColor = water; }
    else if (tank.GetAmount() < config::WaterPerDrop) { status = "AIR HABIS / KE KOLAM"; statusColor = water; }
    else if (m_rangerBlocked) { status = "DIHADANG / CARI CELAH"; statusColor = Color::FromHex(0xF4B376); }
    text("CATATAN PATROLI", 342, y+11, muted);
    text(status, 342, y+29, statusColor);
    text("WASD GERAK / KLIK AIR", 342, y+47, muted);
    text("SCROLL ZOOM / ESC JEDA", w-140, y+11, muted);
    if (m_campaignTime > 0 || m_called) {
        art::DrawFieldIcon(batch, art::FieldIcon::Megaphone, (w-143)*s, (y+29)*s, s, moss);
        const std::string order = m_duty > 0 ? "BANTUAN " + std::to_string(static_cast<int>(std::ceil(m_duty))) + "D"
            : m_called ? "TIM SUDAH DIPANGGIL" : "[Q] PANGGIL";
        text(order, w-130, y+30, moss);
        if (m_duty > 0) {
            rect(w-142, y+47, 126, 5, rule);
            rect(w-142, y+47, 126*m_duty/m_dutyTotal, 5, moss);
        } else if (m_campaignTime > 0) {
            text("SISA " + CampaignClock(m_campaignTime), w-130, y+47, muted);
        }
    }
    // At the standard width these hints share the same rail; wider windows get breathing room.

    if (m_rules.GetOutcome() != StageOutcome::Playing) {
        const bool won = m_rules.GetOutcome() == StageOutcome::Won;
        const std::string title = won ? "OBJEKTIF TERCAPAI" :
            m_rules.GetOutcome() == StageOutcome::LostSawit ? "HUTAN JADI SAWIT" :
            m_rules.GetOutcome() == StageOutcome::LostAnimals ? "TARGET SATWA GAGAL" : "WAKTU HABIS";
        const float tw = static_cast<float>(title.size()) * 18 - 3;
        rect((w-tw)*0.5f-12, h*0.5f-22, tw+24, 40, Color::FromHex(0x152219, 230));
        art::DrawPixelText(batch, title, std::round((w-tw)*0.5f*s), std::round((h*0.5f-12)*s), 3*s,
                           won ? moss : Color::FromHex(0xF5A26B));
    }
    batch.End();
}

} // namespace game
