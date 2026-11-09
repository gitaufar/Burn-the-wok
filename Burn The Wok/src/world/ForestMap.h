#pragma once

#include <engine/math/Rect.h>
#include <engine/math/Vec2.h>

#include <cstdint>
#include <vector>

namespace engine {
class SpriteBatch;
class Texture;
} // namespace engine

namespace game {

enum class TileType : std::uint8_t { Grass, GrassFlowers, Tree, Water };

// Placeholder forest map: a fixed-seed random grid of grass, trees and a pond.
// Trees and water block walking (Stage 2); burning trees come in Stage 3
// (docs/05-implementation-plan.md).
class ForestMap {
public:
    ForestMap(int widthInTiles, int heightInTiles, unsigned seed);

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    TileType GetTile(int x, int y) const; // outside the map counts as Tree
    bool IsPath(int x, int y) const;

    // --- Collision (tile coordinates / world pixels) ---
    // Trees and water cannot be walked through; outside the map is a Tree, so the
    // map edge is solid as well.
    bool IsSolid(int x, int y) const;
    // True when area (world pixels) overlaps any solid tile.
    bool IsBlocked(const engine::Rect& area) const;
    // True when area overlaps at least one tile of the given type. Example: inflate
    // the player's hitbox by a few pixels and ask for Water to detect "next to the pond".
    bool HasTileInArea(const engine::Rect& area, TileType type) const;
    // Rectangle of tile (x, y) in world pixels.
    engine::Rect GetTileRect(int x, int y) const;

    engine::Rect GetBounds() const;  // in world pixels
    engine::Vec2 GetCenter() const;  // in world pixels

    // Only tiles inside visibleArea are drawn.
    void Draw(engine::SpriteBatch& batch, const engine::Texture& atlas,
              const engine::Texture& treeTexture, const engine::Rect& treeSource,
              const engine::Rect& visibleArea, float time = 0.0f) const;

private:
    int m_width = 0;
    int m_height = 0;
    std::vector<TileType> m_tiles; // row-major: index = y * width + x
};

} // namespace game
