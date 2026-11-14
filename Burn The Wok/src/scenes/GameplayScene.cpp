#include "scenes/GameplayScene.h"

#include "GameConfig.h"
#include "art/PlaceholderArt.h"
#include "art/PixelText.h"
#include "entities/Fire.h"
#include "entities/Particle.h"
#include "entities/WaterDrop.h"
#include "scenes/MenuScene.h"
#include "scenes/ResultScene.h"

#include <engine/core/Application.h>
#include <engine/core/Log.h>
#include <engine/graphics/Image.h>
#include <engine/physics/Collision.h>

#include <algorithm>
#include <cmath>
#include <exception>
#include <string>
#include <utility>

namespace game {

using engine::Color;
using engine::Key;
using engine::Vec2;

GameplayScene::GameplayScene()
    : m_map(config::MapWidthTiles, config::MapHeightTiles, config::MapSeed),
      m_player(m_map.GetCenter()),
      m_sprayer(config::FireSeed + 1),
      m_rules(config::StageDuration, config::SawitLoseFraction, m_map.CountTiles(TileType::Tree)),
      m_zoom(config::DefaultZoom) {}

void GameplayScene::OnEnter() {
    m_player.LoadArtwork();
    m_atlas = std::make_unique<engine::Texture>(BuildPlaceholderAtlas());
    m_fireSprite = LoadSprite("assets/items/fire.png");
    m_sawitSprite = LoadSprite("assets/items/palm_oil_tree.png");
    m_waterSprayLeft = LoadSprite("assets/items/water_spray_left.png");
    m_waterSprayRight = LoadSprite("assets/items/water_spray_right.png");
    m_entities.Clear();
    m_fireSystem = std::make_unique<FireSystem>(
        FireArt{m_atlas.get(), m_fireSprite.texture.get(), m_fireSprite.source}, config::FireSeed);
    m_fireSystem->IgniteRandomTrees(m_entities, m_map, config::InitialFireCount);
    const engine::Image tree = engine::Image::LoadFromFile("assets/environment/Tree.png");
    m_treeSource = tree.GetContentBounds(16);
    m_treeTexture = std::make_unique<engine::Texture>(tree);
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
                 "fire_loop", "fire_out", "fire_spread", "tree_to_palm", "timer_warning"});
    try {
        m_music = App().GetAudio().LoadSound("assets/audio/music/gameplay.wav");
        m_musicVoice = App().GetAudio().Play(m_music, 0.0f, true); // faded in from UpdateSound()
    } catch (const std::exception& e) {
        engine::LogError("Gameplay music unavailable: %s", e.what()); // the game still works in silence
    }
    m_sfx->Play("stage_start", config::sound::StageStart);

    App().GetWindow().SetTitle(std::string(config::WindowTitle) +
                               " - Gameplay | WASD/panah: gerak, klik kiri: semprot, tepi kolam: isi air, "
                               "F1: menang, F2: kalah, F3: hitbox, F4: nyalakan api, Esc: menu");
    App().GetWindow().SetClearColor(Color::FromHex(0x1B3027));
}

void GameplayScene::OnExit() {
    StopSounds();
}

void GameplayScene::StopSounds() {
    engine::AudioSystem& audio = App().GetAudio();
    for (engine::AudioSystem::VoiceId* voice : {&m_musicVoice, &m_sprayVoice, &m_fireVoice, &m_tickVoice}) {
        audio.Stop(*voice);
        *voice = engine::AudioSystem::InvalidVoice;
    }
}

void GameplayScene::Update(float dt) {
    m_time += dt;
    const engine::Input& input = App().GetInput();

    if (input.IsKeyPressed(Key::Escape)) {
        App().GetScenes().SwitchTo(std::make_unique<MenuScene>());
        return;
    }
    // Debug shortcuts until real win/lose rules exist (Stage 3).
    if (input.IsKeyPressed(Key::F1)) {
        App().GetScenes().SwitchTo(std::make_unique<ResultScene>(ResultScene::Outcome::Win));
        return;
    }
    if (input.IsKeyPressed(Key::F2)) {
        App().GetScenes().SwitchTo(std::make_unique<ResultScene>(ResultScene::Outcome::Lose));
        return;
    }

    if (const int wheel = input.GetMouseWheel(); wheel != 0) {
        m_zoom = std::clamp(m_zoom + wheel, config::MinZoom, config::MaxZoom);
    }

    // Debug: show hitboxes and solid tiles (useful when tuning collision).
    if (input.IsKeyPressed(Key::F3)) {
        m_showCollision = !m_showCollision;
    }

    m_player.Update(input, dt, m_map);

    // Spray toward the mouse. The camera is from the last drawn frame, which is what
    // the player is looking at when they click.
    const Vec2 mouseWorld = m_camera.ScreenToWorld(input.GetMousePosition());
    const Vec2 aim = (mouseWorld - m_player.GetPosition()).Normalized();
    const bool trigger = input.IsMouseDown(engine::MouseButton::Left);
    if (trigger) {
        m_player.Aim(aim);
        if (aim != Vec2{}) m_aim = aim;
    }

    m_sprayer.Update(dt, trigger, m_player.GetNozzlePosition(aim), aim, m_player.GetTank(), m_entities);

    // Debug: light the tree under the cursor (fires do not spread until Stage 3).
    if (input.IsKeyPressed(Key::F4)) {
        const float ts = static_cast<float>(config::TileSize);
        m_fireSystem->Ignite(m_entities, m_map, static_cast<int>(std::floor(mouseWorld.x / ts)),
                             static_cast<int>(std::floor(mouseWorld.y / ts)));
    }

    m_entities.Update(dt);
    const int putOut = m_fireSystem->ResolveWaterHits(m_entities);
    m_firesPutOut += putOut;
    const FireSystem::StepResult step = m_fireSystem->Update(m_entities, m_map, dt);
    if (m_map.GetRevision() != m_minimapRevision) {
        RebuildMinimap();
    }

    // The outcome is shown in the world for a moment before the result screen.
    const StageOutcome outcome =
        m_rules.Update(dt, m_entities.Count<Fire>(), m_map.CountTiles(TileType::Sawit));
    UpdateSound(dt, trigger, putOut, step, outcome);
    if (outcome != StageOutcome::Playing) {
        m_endTimer += dt;
        if (m_endTimer >= config::ResultDelay) {
            FinishStage();
        }
    }
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
    case StageOutcome::Won: summary.reason = "SEMUA API PADAM"; break;
    case StageOutcome::LostSawit: summary.reason = "LAHAN SAWIT MELEWATI BATAS"; break;
    case StageOutcome::LostTime: summary.reason = "WAKTU HABIS - API MASIH MENYALA"; break;
    case StageOutcome::Playing: break;
    }
    summary.hasStats = true;
    summary.forestPercent = static_cast<int>(std::round(m_rules.GetForestFraction() * 100.0f));
    summary.sawitPercent = static_cast<int>(std::round(m_rules.GetSawitFraction() * 100.0f));
    summary.firesPutOut = m_firesPutOut;
    summary.secondsLeft = static_cast<int>(std::ceil(m_rules.GetTimeLeft()));
    const auto result = m_rules.GetOutcome() == StageOutcome::Won ? ResultScene::Outcome::Win
                                                                  : ResultScene::Outcome::Lose;
    App().GetScenes().SwitchTo(std::make_unique<ResultScene>(result, std::move(summary)));
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
    const Vec2 viewport = App().GetWindow().GetSize();
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
    UpdateCamera(alpha);

    engine::SpriteBatch& batch = App().GetSpriteBatch();
    batch.Begin(m_camera.GetViewProjection());

    const auto visibleArea = m_camera.GetVisibleArea();
    const float playerFeetY = m_player.GetRenderFeetY(alpha);
    m_map.DrawGround(batch, *m_atlas, visibleArea, m_time);
    m_map.DrawTrees(batch, *m_treeTexture, m_treeSource, *m_sawitSprite.texture, m_sawitSprite.source,
                    visibleArea, playerFeetY, false);
    DrawFires(alpha, playerFeetY, false);
    m_player.Draw(batch, *m_atlas, alpha, static_cast<float>(m_zoom));
    DrawWaterBurst(alpha);
    m_map.DrawTrees(batch, *m_treeTexture, m_treeSource, *m_sawitSprite.texture, m_sawitSprite.source,
                    visibleArea, playerFeetY, true);
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
    const Vec2 mouseWorld = m_camera.ScreenToWorld(App().GetInput().GetMousePosition());
    const float ts = static_cast<float>(config::TileSize);
    const engine::Rect hovered{std::floor(mouseWorld.x / ts) * ts, std::floor(mouseWorld.y / ts) * ts,
                               ts, ts};
    if (m_map.GetBounds().Contains(mouseWorld)) {
        const Color marker = Color::FromHex(0xF2DEA0, 130);
        batch.DrawRect({hovered.x, hovered.y, 4, 1}, marker);
        batch.DrawRect({hovered.x, hovered.y, 1, 4}, marker);
        batch.DrawRect({hovered.Right() - 4, hovered.Bottom() - 1, 4, 1}, marker);
        batch.DrawRect({hovered.Right() - 1, hovered.Bottom() - 4, 1, 4}, marker);
    }

    if (m_showCollision) {
        DrawCollisionDebug(alpha);
    }

    batch.End();
    DrawOverlay(alpha);
}

void GameplayScene::DrawOverlay(float alpha) {
    auto& batch = App().GetSpriteBatch();
    const Vec2 size = App().GetWindow().GetSize();
    const float fit = std::min(size.x / 640.0f, size.y / 360.0f);
    const float s = fit >= 1 ? std::floor(fit) : std::max(0.25f, fit);
    const float w = size.x / s, h = size.y / s;
    engine::Camera2D camera;
    camera.SetViewportSize(size);
    camera.SetPosition(size * 0.5f);
    batch.Begin(camera.GetViewProjection());
    batch.Draw(*m_vignette, {0, 0, size.x, size.y});
    const Color edge = Color::FromHex(0x29271F);
    const Color paper = Color::FromHex(0xE8D9AF);
    const Color shade = Color::FromHex(0xC7B58A), ink = Color::FromHex(0x3B4031);
    const Color cream = Color::FromHex(0xFFF0CB), moss = Color::FromHex(0x738650);
    auto rect = [&](float x, float y, float width, float height, Color color) {
        batch.DrawRect({std::round(x*s), std::round(y*s), std::round(width*s), std::round(height*s)}, color);
    };
    auto text = [&](std::string_view value, float x, float y, Color color) {
        art::DrawPixelText(batch, value, std::round(x*s), std::round(y*s), s, color);
    };
    // Painted trail badge and open location typography.
    rect(14, 14, 29, 37, edge);
    rect(16, 14, 25, 33, moss);
    rect(20, 47, 17, 3, moss);
    rect(27, 25, 3, 16, cream);
    rect(25, 20, 7, 3, cream);
    rect(22, 23, 13, 4, cream);
    rect(20, 27, 17, 4, cream);
    rect(23, 31, 11, 3, cream);
    rect(51, 15, 110, 12, Color::FromHex(0x152219, 210));
    text("KAWASAN KONSERVASI", 55, 18, shade);
    art::DrawPixelText(batch, "HUTAN TROPIS", 52*s, 33*s, 2*s, edge);
    art::DrawPixelText(batch, "HUTAN TROPIS", 51*s, 31*s, 2*s, cream);
    rect(52, 51, 22, 2, moss);
    rect(77, 51, 114, 1, Color::FromHex(0xFFF0CB, 100));

    // Water tank gauge and fire counter, under the location title.
    {
        const WaterTank& tank = m_player.GetTank();
        const float hx = 14, hy = 60;
        rect(hx, hy, 182, 44, Color::FromHex(0x152219, 195));
        text("AIR", hx+6, hy+6, cream);
        rect(hx+28, hy+5, 104, 9, edge);
        rect(hx+30, hy+7, 100, 5, Color::FromHex(0x2A3A33));
        const float fill = std::round(100.0f * tank.GetFraction());
        const Color water = tank.GetFraction() < 0.2f ? Color::FromHex(0xE07A4F) : Color::FromHex(0x5FB8E0);
        rect(hx+30, hy+7, fill, 5, water);
        if (fill > 0) rect(hx+30, hy+7, fill, 1, Color::FromHex(0xD8F3FF, 160));
        text(std::to_string(static_cast<int>(fill)), hx+138, hy+6, cream);

        const bool empty = tank.GetAmount() < config::WaterPerDrop;
        if (m_player.IsRefilling()) {
            text("MENGISI ULANG", hx+6, hy+19, Color::FromHex(0x8FD8F0));
        } else if (empty) {
            if (std::fmod(m_time, 0.8f) < 0.55f) text("AIR HABIS - KE KOLAM", hx+6, hy+19, Color::FromHex(0xF08A5D));
        } else {
            text("KLIK KIRI: SEMPROT", hx+6, hy+19, shade);
        }

        const int fires = m_entities.Count<Fire>();
        if (fires > 0) {
            const float iconW = std::round(10.0f * m_fireSprite.source.w / m_fireSprite.source.h);
            batch.Draw(*m_fireSprite.texture, {std::round((hx+6)*s), std::round((hy+28)*s), iconW*s, 10*s},
                       m_fireSprite.source);
            text("API: " + std::to_string(fires), hx+17, hy+31, Color::FromHex(0xFFC14D));
            text("PADAM: " + std::to_string(m_firesPutOut), hx+84, hy+31, shade);
        } else {
            text("SEMUA API PADAM", hx+6, hy+31, Color::FromHex(0xB6D98A));
        }
    }

    // Stage clock and land status, top center.
    {
        const float tx = std::round(w * 0.5f) - 62, ty = 14;
        rect(tx, ty, 124, 44, Color::FromHex(0x152219, 195));
        const int seconds = static_cast<int>(std::ceil(m_rules.GetTimeLeft()));
        const std::string clock = std::to_string(seconds / 60) + ":" +
                                  (seconds % 60 < 10 ? "0" : "") + std::to_string(seconds % 60);
        const bool hurry = seconds <= 30 && std::fmod(m_time, 1.0f) < 0.5f;
        art::DrawPixelText(batch, clock,
                           std::round((tx + 62 - (static_cast<float>(clock.size()) * 12 - 2) * 0.5f) * s),
                           std::round((ty + 5) * s), 2 * s, hurry ? Color::FromHex(0xF08A5D) : cream);
        const int forest = static_cast<int>(std::round(m_rules.GetForestFraction() * 100.0f));
        const int sawit = static_cast<int>(std::round(m_rules.GetSawitFraction() * 100.0f));
        const int limit = static_cast<int>(std::round(m_rules.GetSawitLoseFraction() * 100.0f));
        text("HUTAN " + std::to_string(forest) + "%", tx + 6, ty + 23, Color::FromHex(0xB6D98A));
        text(std::to_string(sawit) + "/" + std::to_string(limit) + "%", tx + 76, ty + 23, shade);
        // Sawit share as a bar toward the losing limit.
        text("SAWIT", tx + 6, ty + 33, Color::FromHex(0xE0B54A));
        rect(tx + 40, ty + 33, 78, 7, edge);
        rect(tx + 41, ty + 34, 76, 5, Color::FromHex(0x2A3A33));
        const float fill = std::min(1.0f, m_rules.GetSawitFraction() / m_rules.GetSawitLoseFraction());
        rect(tx + 41, ty + 34, std::round(76 * fill), 5,
             fill > 0.66f ? Color::FromHex(0xE0503C) : Color::FromHex(0xC9A23A));
    }

    // Outcome banner during the short pause before the result screen.
    if (m_rules.GetOutcome() != StageOutcome::Playing) {
        const bool won = m_rules.GetOutcome() == StageOutcome::Won;
        const std::string title = won ? "SEMUA API PADAM" :
            m_rules.GetOutcome() == StageOutcome::LostSawit ? "HUTAN JADI SAWIT" : "WAKTU HABIS";
        const float textWidth = static_cast<float>(title.size()) * 18 - 3;
        rect(std::round((w - textWidth) * 0.5f) - 12, std::round(h * 0.5f) - 22, textWidth + 24, 40,
             Color::FromHex(0x152219, 220));
        art::DrawPixelText(batch, title, std::round((w - textWidth) * 0.5f * s),
                           std::round((h * 0.5f - 12) * s), 3 * s,
                           won ? Color::FromHex(0xB6D98A) : Color::FromHex(0xF5A26B));
    }

    // Folded paper chart with tape tabs, compass and coordinate margin.
    const float mx = w-151, my = 17;
    rect(mx+4, my+5, 132, 131, Color::FromHex(0x152219, 130));
    rect(mx, my, 130, 128, paper);
    rect(mx, my, 4, 128, shade);
    rect(mx+128, my+3, 3, 122, shade);
    rect(mx+8, my-3, 28, 7, Color::FromHex(0xB9B58A));
    rect(mx+92, my-3, 28, 7, Color::FromHex(0xB9B58A));
    text("PETA", mx+11, my+10, ink);
    text("LAPANGAN", mx+11, my+21, ink);
    text("U", mx+114, my+8, ink);
    rect(mx+116, my+18, 1, 10, ink);
    rect(mx+114, my+20, 5, 1, ink);
    rect(mx+115, my+19, 3, 1, ink);
    rect(mx+11, my+33, 108, 1, shade);
    const engine::Rect mapRect{(mx+11)*s, (my+40)*s, 108*s, 72*s};
    rect(mx+10, my+39, 110, 74, ink);
    batch.Draw(*m_minimap, mapRect);
    rect(mx+47, my+40, 1, 72, Color::FromHex(0xFFF0CB, 35));
    rect(mx+83, my+40, 1, 72, Color::FromHex(0x29271F, 25));
    const auto world = m_map.GetBounds();
    const auto view = m_camera.GetVisibleArea();
    const float left = std::clamp(view.Left(), 0.0f, world.w);
    const float right = std::clamp(view.Right(), 0.0f, world.w);
    const float top = std::clamp(view.Top(), 0.0f, world.h);
    const float bottom = std::clamp(view.Bottom(), 0.0f, world.h);
    batch.DrawRectOutline({std::round(mapRect.x + left/world.w*mapRect.w),
        std::round(mapRect.y + top/world.h*mapRect.h),
        std::round((right-left)/world.w*mapRect.w),
        std::round((bottom-top)/world.h*mapRect.h)}, s, Color::FromHex(0xFFF0CB, 150));
    const bool blink = std::fmod(m_time, 0.6f) < 0.4f;
    std::as_const(m_entities).ForEach<Fire>([&](const Fire& fire) {
        const float fx = mx+11+std::round(fire.GetPosition().x/world.w*108);
        const float fy = my+40+std::round((fire.GetPosition().y-8)/world.h*72);
        rect(fx-1, fy-1, 3, 3, blink ? Color::FromHex(0xFF5A2A) : Color::FromHex(0xFFC14D));
    });
    const Vec2 player = m_player.GetRenderPosition(alpha);
    const float px = mx+11+std::round(player.x/world.w*108);
    const float py = my+40+std::round(player.y/world.h*72);
    rect(px-2, py-2, 5, 5, edge);
    rect(px-1, py-1, 3, 3, cream);
    rect(px, py, 1, 1, Color::FromHex(0xB45D41));
    switch (m_player.GetFacing()) {
    case Facing::Up: rect(px, py-4, 1, 2, cream); break;
    case Facing::Down: rect(px, py+3, 1, 2, cream); break;
    case Facing::Left: rect(px-4, py, 2, 1, cream); break;
    case Facing::Right: rect(px+3, py, 2, 1, cream); break;
    }
    auto coordinate = [](int value) {
        return (value < 10 ? std::string("0") : std::string()) + std::to_string(value);
    };
    text("X:" + coordinate(static_cast<int>(player.x/config::TileSize)) +
         " Y:" + coordinate(static_cast<int>(player.y/config::TileSize)), mx+11, my+118, ink);
    text(std::to_string(m_zoom)+"X", mx+106, my+118, ink);

    // Compact physical keycaps react to both WASD and the arrow keys.
    const float by = h-51;
    rect(10, by-4, 322, 45, Color::FromHex(0x152219, 195));
    const auto& input = App().GetInput();
    auto keycap = [&](std::string_view key, float x, float y, float width, bool down) {
        rect(x, y+2, width, 12, edge);
        rect(x, y+(down ? 2 : 0), width, 11, down ? moss : paper);
        if (!down) rect(x+1, y+1, width-2, 1, cream);
        text(key, x+(width-static_cast<float>(key.size())*6+1)*0.5f,
             y+(down ? 4 : 2), down ? cream : ink);
    };
    keycap("W", 32, by, 13, input.IsKeyDown(Key::W) || input.IsKeyDown(Key::Up));
    keycap("A", 17, by+15, 13, input.IsKeyDown(Key::A) || input.IsKeyDown(Key::Left));
    keycap("S", 32, by+15, 13, input.IsKeyDown(Key::S) || input.IsKeyDown(Key::Down));
    keycap("D", 47, by+15, 13, input.IsKeyDown(Key::D) || input.IsKeyDown(Key::Right));
    text("JELAJAH", 69, by+5, cream);
    text("WASD / PANAH", 69, by+19, shade);
    rect(150, by+2, 1, 26, Color::FromHex(0xFFF0CB, 60));
    keycap("ESC", 162, by+3, 23, input.IsKeyDown(Key::Escape));
    text("MENU", 192, by+6, cream);
    text("SCROLL: ZOOM", 162, by+21, shade);
    rect(240, by+2, 1, 26, Color::FromHex(0xFFF0CB, 60));
    // Mouse with the left button lit while spraying.
    const bool sprayDown = input.IsMouseDown(engine::MouseButton::Left);
    rect(252, by+3, 11, 16, edge);
    rect(253, by+4, 9, 14, paper);
    rect(253, by+4, 4, 6, sprayDown ? Color::FromHex(0x5FB8E0) : shade);
    rect(257, by+4, 1, 6, edge);
    text("SEMPROT", 270, by+5, cream);
    text("KLIK KIRI", 270, by+19, shade);
    batch.End();
}

} // namespace game
