#include "scenes/GameplayScene.h"

#include "GameConfig.h"
#include "art/PlaceholderArt.h"
#include "scenes/MenuScene.h"
#include "scenes/ResultScene.h"

#include <engine/core/Application.h>

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

    App().GetWindow().SetTitle(std::string(config::WindowTitle) +
                               " - Gameplay | WASD/panah: gerak, roda mouse: zoom, "
                               "F1: menang, F2: kalah, Esc: menu");
    App().GetWindow().SetClearColor(Color::Black());
}

void GameplayScene::Update(float dt) {
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

    m_player.Update(input, dt, m_map.GetBounds());
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

    m_map.Draw(batch, *m_atlas, m_camera.GetVisibleArea());
    m_player.Draw(batch, *m_atlas, alpha);

    // Highlight the tile under the mouse: shows the screen -> world conversion working.
    const Vec2 mouseWorld = m_camera.ScreenToWorld(App().GetInput().GetMousePosition());
    const float ts = static_cast<float>(config::TileSize);
    const engine::Rect hovered{std::floor(mouseWorld.x / ts) * ts, std::floor(mouseWorld.y / ts) * ts,
                               ts, ts};
    if (m_map.GetBounds().Contains(mouseWorld)) {
        batch.DrawRectOutline(hovered, 1.0f, Color::White().WithAlpha(170));
    }

    batch.End();
}

} // namespace game
