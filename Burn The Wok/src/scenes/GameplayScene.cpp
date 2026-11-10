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
    m_player.LoadArtwork();
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

    const auto visibleArea = m_camera.GetVisibleArea();
    const float playerFeetY = m_player.GetRenderFeetY(alpha);
    m_map.DrawGround(batch, *m_atlas, visibleArea, m_time);
    m_map.DrawTrees(batch, *m_treeTexture, m_treeSource, visibleArea, playerFeetY, false);
    m_player.Draw(batch, *m_atlas, alpha, static_cast<float>(m_zoom));
    m_map.DrawTrees(batch, *m_treeTexture, m_treeSource, visibleArea, playerFeetY, true);

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
    rect(10, by-4, 240, 45, Color::FromHex(0x152219, 195));
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
    text("SCROLL: ZOOM", 162, by+21, shade);    batch.End();
}

} // namespace game
