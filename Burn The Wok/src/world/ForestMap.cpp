#include "world/ForestMap.h"

#include "GameConfig.h"
#include "art/PlaceholderArt.h"

#include <engine/graphics/SpriteBatch.h>
#include <engine/graphics/Texture.h>

#include <algorithm>
#include <cmath>
#include <random>

namespace game {

ForestMap::ForestMap(int widthInTiles, int heightInTiles, unsigned seed)
    : m_width(widthInTiles), m_height(heightInTiles) {
    m_tiles.resize(static_cast<std::size_t>(m_width) * m_height, TileType::Grass);

    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> chance(0.0f, 1.0f);

    const float centerX = m_width * 0.5f;
    const float centerY = m_height * 0.5f;
    const float pondX = m_width * 0.72f;
    const float pondY = m_height * 0.3f;

    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            TileType tile = TileType::Grass;
            const float roll = chance(rng); // always roll, so the layout only depends on the seed

            const bool border = x == 0 || y == 0 || x == m_width - 1 || y == m_height - 1;
            const float cdx = x - centerX;
            const float cdy = y - centerY;
            const bool spawnClearing = cdx * cdx + cdy * cdy < 16.0f;
            const float pdx = (x - pondX) / 5.0f;
            const float pdy = (y - pondY) / 3.0f;
            const bool pond = pdx * pdx + pdy * pdy <= 1.0f;

            if (border) {
                tile = TileType::Tree;
            } else if (pond) {
                tile = TileType::Water;
            } else if (spawnClearing) {
                tile = TileType::Grass;
            } else if (roll < 0.12f) {
                tile = TileType::Tree;
            } else if (roll < 0.25f) {
                tile = TileType::GrassFlowers;
            }
            m_tiles[static_cast<std::size_t>(y) * m_width + x] = tile;
        }
    }
}

TileType ForestMap::GetTile(int x, int y) const {
    if (x < 0 || y < 0 || x >= m_width || y >= m_height) {
        return TileType::Tree;
    }
    return m_tiles[static_cast<std::size_t>(y) * m_width + x];
}

engine::Rect ForestMap::GetBounds() const {
    return {0.0f, 0.0f, static_cast<float>(m_width * config::TileSize),
            static_cast<float>(m_height * config::TileSize)};
}

engine::Vec2 ForestMap::GetCenter() const {
    return GetBounds().Center();
}

bool ForestMap::IsPath(int x, int y) const {
    const TileType tile = GetTile(x, y);
    if (tile != TileType::Grass && tile != TileType::GrassFlowers) return false;
    const float cx = m_width * 0.5f;
    const float cy = m_height * 0.5f;
    const float dx = x - cx;
    const float dy = y - cy;
    const float trailY = cy + std::sin((x - cx) * 0.24f) * 1.4f;
    const float trailX = m_width * 0.72f + std::sin(y * 0.31f) * 0.65f;
    return dx * dx + dy * dy < 5.0f ||
           (x > cx - 2 && x < m_width - 3 && std::abs(y - trailY) < 0.9f) ||
           (y > m_height * 0.3f + 2 && y < cy + 1 && std::abs(x - trailX) < 0.8f);
}

void ForestMap::Draw(engine::SpriteBatch& batch, const engine::Texture& atlas,
                     const engine::Texture& treeTexture, const engine::Rect& treeSource,
                     const engine::Rect& visibleArea, float time) const {
    const float ts = static_cast<float>(config::TileSize);

    // Convert the visible world rectangle to a tile range, so a big map costs
    // only as much as what fits on screen.
    const int x0 = std::max(0, static_cast<int>(std::floor(visibleArea.Left() / ts)));
    const int y0 = std::max(0, static_cast<int>(std::floor(visibleArea.Top() / ts)));
    const int x1 = std::min(m_width - 1, static_cast<int>(std::floor(visibleArea.Right() / ts)));
    const int y1 = std::min(m_height - 1, static_cast<int>(std::floor(visibleArea.Bottom() / ts)));

    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            const engine::Rect dest{x * ts, y * ts, ts, ts};
            const unsigned variation = static_cast<unsigned>(x * 73 + y * 131);
            const float patch = std::sin(x * 0.43f + y * 0.27f) * std::cos(y * 0.38f);
            const auto shade = static_cast<std::uint8_t>(243.0f + patch * 12.0f);
            const engine::Color tint{shade, shade, static_cast<std::uint8_t>(shade - 4), 255};
            const SpriteId grass = variation % 3 == 0 ? SpriteId::GrassSoft :
                                   variation % 3 == 1 ? SpriteId::Grass : SpriteId::GrassDense;
            switch (GetTile(x, y)) {
            case TileType::Grass:
                batch.Draw(atlas, dest, GetSpriteSource(IsPath(x, y) ? SpriteId::Dirt : grass), tint);
                break;
            case TileType::GrassFlowers:
                batch.Draw(atlas, dest, GetSpriteSource(IsPath(x, y) ? SpriteId::Dirt : SpriteId::GrassFlowers), tint);
                break;
            case TileType::Tree:
                // Trees have transparent corners, so draw the ground underneath first.
                batch.Draw(atlas, dest, GetSpriteSource(grass), tint);
                break;
            case TileType::Water:
                {
                    const SpriteId frames[] = {SpriteId::Water, SpriteId::WaterRipple,
                        SpriteId::WaterRippleTwo, SpriteId::WaterRippleThree};
                    const int frame = (static_cast<int>(time * 2.5f) + x / 2 + y / 3) % 4;
                    batch.Draw(atlas, dest, GetSpriteSource(frames[frame]));
                }
                break;
            }
        }
    }

    // Shoreline and path edges break up the square tile grid.
    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            const float px = x * ts, py = y * ts;
            if (GetTile(x, y) == TileType::Water) {
                const engine::Color bank = engine::Color::FromHex(0xB3AE75);
                const engine::Color foam = engine::Color::FromHex(0xA0C5AC, 150);
                if (GetTile(x, y - 1) != TileType::Water) {
                    batch.DrawRect({px, py, ts, 2}, bank);
                    batch.DrawRect({px + 1, py + 2, ts - 2, 1}, foam);
                }
                if (GetTile(x, y + 1) != TileType::Water) {
                    batch.DrawRect({px, py + ts - 2, ts, 2}, bank);
                    batch.DrawRect({px + 1, py + ts - 3, ts - 2, 1}, foam);
                }
                if (GetTile(x - 1, y) != TileType::Water) batch.DrawRect({px, py, 2, ts}, bank);
                if (GetTile(x + 1, y) != TileType::Water) batch.DrawRect({px + ts - 2, py, 2, ts}, bank);
            } else if (IsPath(x, y)) {
                const engine::Color edge = engine::Color::FromHex(0x819151);
                if (!IsPath(x, y - 1)) batch.DrawRect({px, py, ts, 1}, edge);
                if (!IsPath(x, y + 1)) batch.DrawRect({px, py + ts - 1, ts, 1}, edge);
                if (!IsPath(x - 1, y)) batch.DrawRect({px, py, 1, ts}, edge);
                if (!IsPath(x + 1, y)) batch.DrawRect({px + ts - 1, py, 1, ts}, edge);
            }
        }
    }

    // Reuse small procedural sprites for shadows, stones and waterside plants.
    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            const TileType tile = GetTile(x, y);
            const unsigned detail = static_cast<unsigned>(x * 317 + y * 167);
            if (tile == TileType::Tree) {
                batch.Draw(atlas, {x * ts + 2, y * ts + 11, ts - 1, 5}, GetSpriteSource(SpriteId::Shadow));
            } else if (tile != TileType::Water && !IsPath(x, y)) {
                const bool nearWater = GetTile(x + 1, y) == TileType::Water ||
                    GetTile(x - 1, y) == TileType::Water || GetTile(x, y + 1) == TileType::Water ||
                    GetTile(x, y - 1) == TileType::Water;
                if (nearWater && detail % 3 != 0) {
                    batch.Draw(atlas, {x * ts, y * ts, ts, ts}, GetSpriteSource(SpriteId::Reeds));
                } else if (tile == TileType::Grass && detail % 23 == 0) {
                    batch.Draw(atlas, {x * ts, y * ts, ts, ts}, GetSpriteSource(SpriteId::Rock));
                } else if (tile == TileType::Grass && detail % 19 == 0) {
                    batch.Draw(atlas, {x * ts, y * ts, ts, ts}, GetSpriteSource(SpriteId::Shrub));
                }
            }
        }
    }

    // Draw all trees together to avoid switching textures for each individual tile.
    // Fit the visible artwork to one tile and align its roots to the tile bottom.
    const float scale = ts / std::max(treeSource.w, treeSource.h);
    const float width = treeSource.w * scale;
    const float height = treeSource.h * scale;
    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            if (GetTile(x, y) == TileType::Tree) {
                batch.Draw(treeTexture,
                           {x * ts + (ts - width) * 0.5f, y * ts + ts - height, width, height},
                           treeSource, engine::Color::FromHex(
                               (x * 13 + y * 7) % 3 == 0 ? 0xDEE4CA : 0xF2EDDC));
            }
        }
    }
}

} // namespace game
