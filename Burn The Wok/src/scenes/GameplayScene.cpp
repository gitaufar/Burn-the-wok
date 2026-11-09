#include "scenes/GameplayScene.h"

#include "GameConfig.h"
#include "art/PlaceholderArt.h"
#include "art/PixelText.h"
#include "scenes/MenuScene.h"
#include "scenes/ResultScene.h"

#include <engine/core/Application.h>
#include <engine/graphics/Image.h>
#include <engine/physics/Collision.h>

#include <algorithm>
#include <cmath>
#include <string>

namespace game {

using engine::Color;
using engine::Key;
using engine::Vec2;

GameplayScene::GameplayScene()
    : m_map(config::MapWidthTiles, config::MapHeightTiles, config::MapSeed),
      m_player(m_map.GetCenter()),
      m_zoom(config::DefaultZoom) {}

void GameplayScene::OnEnter() {
    m_atlas = std::make_unique<engine::Texture>(BuildPlaceholderAtlas());
    const engine::Image tree = engine::Image::LoadFromFile("assets/environment/Tree.png");
    m_treeSource = tree.GetContentBounds(16);
    m_treeTexture = std::make_unique<engine::Texture>(tree);

    engine::Image minimap(m_map.GetWidth(), m_map.GetHeight());
    for (int y = 0; y < m_map.GetHeight(); ++y) {
        for (int x = 0; x < m_map.GetWidth(); ++x) {
            Color color = Color::FromHex(0x708457);
            switch (m_map.GetTile(x, y)) {
            case TileType::Tree: color = Color::FromHex(0x36573B); break;
            case TileType::Water: color = Color::FromHex(0x5AADB0); break;
            default: if (m_map.IsPath(x, y)) color = Color::FromHex(0xC8AF7D); break;
            }
            minimap.SetPixel(x, y, color);
        }
    }
    m_minimap = std::make_unique<engine::Texture>(minimap);

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

    App().GetWindow().SetTitle(std::string(config::WindowTitle) +
                               " - Gameplay | WASD/panah: gerak, roda mouse: zoom, "
                               "F1: menang, F2: kalah, F3: hitbox, Esc: menu");
    App().GetWindow().SetClearColor(Color::FromHex(0x1B3027));
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

    m_map.Draw(batch, *m_atlas, *m_treeTexture, m_treeSource, m_camera.GetVisibleArea(), m_time);
    m_player.Draw(batch, *m_atlas, alpha);

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
    engine::SpriteBatch& batch = App().GetSpriteBatch();
    const Vec2 size = App().GetWindow().GetSize();
    const float s = std::max(0.1f, std::min(1.0f, std::min(size.x / 960.0f, size.y / 600.0f)));
    engine::Camera2D camera;
    camera.SetViewportSize(size);
    camera.SetPosition(size * 0.5f);
    batch.Begin(camera.GetViewProjection());
    batch.Draw(*m_vignette, {0, 0, size.x, size.y});

    const Color ink = Color::FromHex(0x102A23, 235);
    const Color border = Color::FromHex(0x617458, 190);
    const Color cream = Color::FromHex(0xEBE4C9);
    const Color muted = Color::FromHex(0xA8BA98);
    const Color gold = Color::FromHex(0xE6BC70);
    auto panel = [&](const engine::Rect& rect) {
        batch.DrawRect({rect.x + 3 * s, rect.y + 4 * s, rect.w, rect.h}, Color::FromHex(0x0D1F19, 90));
        batch.DrawRect(rect, ink);
        batch.DrawRectOutline(rect, s, border);
    };
    auto label = [&](std::string_view text, float x, float y, float scale, Color color) {
        art::DrawPixelText(batch, text, x, y, scale * s, color);
    };

    panel({24 * s, 24 * s, 312 * s, 88 * s});
    batch.DrawRect({24 * s, 24 * s, 3 * s, 88 * s}, gold);
    label("KAWASAN KONSERVASI", 42 * s, 39 * s, 1.1f, muted);
    label("HUTAN TROPIS", 42 * s, 58 * s, 2.1f, cream);
    batch.DrawRect({42 * s, 87 * s, 4 * s, 4 * s}, Color::FromHex(0x9FCB80));
    label("PATROLI HUTAN", 54 * s, 85 * s, 1.1f, muted);

    const float mapX = size.x - 208 * s;
    panel({mapX, 24 * s, 184 * s, 158 * s});
    label("PETA HUTAN", mapX + 16 * s, 39 * s, 1.2f, cream);
    const engine::Rect mapRect{mapX + 16 * s, 60 * s, 152 * s, 101.333f * s};
    batch.Draw(*m_minimap, mapRect);
    const engine::Rect world = m_map.GetBounds();
    const engine::Rect view = m_camera.GetVisibleArea();
    const float left = std::clamp(view.Left(), 0.0f, world.w);
    const float right = std::clamp(view.Right(), 0.0f, world.w);
    const float top = std::clamp(view.Top(), 0.0f, world.h);
    const float bottom = std::clamp(view.Bottom(), 0.0f, world.h);
    batch.DrawRectOutline({mapRect.x + left / world.w * mapRect.w,
                           mapRect.y + top / world.h * mapRect.h,
                           (right - left) / world.w * mapRect.w,
                           (bottom - top) / world.h * mapRect.h}, s, Color::FromHex(0xE4D9A7, 170));
    const Vec2 player = m_player.GetRenderPosition(alpha);
    const float px = mapRect.x + player.x / world.w * mapRect.w;
    const float py = mapRect.y + player.y / world.h * mapRect.h;
    batch.DrawRect({px - 3 * s, py - 3 * s, 6 * s, 6 * s}, ink);
    batch.DrawRect({px - 2 * s, py - 2 * s, 4 * s, 4 * s}, gold);

    // Show real position and zoom instead of meters for unimplemented mechanics.
    const std::string location = "X " + std::to_string(static_cast<int>(player.x / config::TileSize)) +
                                 "  Y " + std::to_string(static_cast<int>(player.y / config::TileSize)) +
                                 "  ZOOM " + std::to_string(m_zoom) + "X";
    panel({24 * s, size.y - 102 * s, 282 * s, 30 * s});
    label(location, 38 * s, size.y - 91 * s, 1.2f, cream);
    panel({24 * s, size.y - 60 * s, 512 * s, 36 * s});
    label("WASD GERAK   SCROLL ZOOM   ESC MENU", 40 * s, size.y - 47 * s, 1.2f, muted);

    const Vec2 screen = m_camera.WorldToScreen(player);
    const float markerY = screen.y - 34.0f * m_zoom / 3.0f -
                          std::round(std::sin(m_time * 2.0f) * 2.0f);
    batch.DrawRect({screen.x - 3 * s, markerY, 6 * s, 2 * s}, gold);
    batch.DrawRect({screen.x - 2 * s, markerY + 2 * s, 4 * s, 2 * s}, gold);
    batch.DrawRect({screen.x - s, markerY + 4 * s, 2 * s, 2 * s}, gold);
    batch.End();
}

} // namespace game
