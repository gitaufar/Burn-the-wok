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

void ForestMap::Draw(engine::SpriteBatch& batch, const engine::Texture& atlas,
                     const engine::Rect& visibleArea) const {
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
            switch (GetTile(x, y)) {
            case TileType::Grass:
                batch.Draw(atlas, dest, GetSpriteSource(SpriteId::Grass));
                break;
            case TileType::GrassFlowers:
                batch.Draw(atlas, dest, GetSpriteSource(SpriteId::GrassFlowers));
                break;
            case TileType::Tree:
                // Trees have transparent corners, so draw the ground underneath first.
                batch.Draw(atlas, dest, GetSpriteSource(SpriteId::Grass));
                batch.Draw(atlas, dest, GetSpriteSource(SpriteId::Tree));
                break;
            case TileType::Water:
                batch.Draw(atlas, dest, GetSpriteSource(SpriteId::Water));
                break;
            }
        }
    }
}

} // namespace game
