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

// Dirt = bare ground (the walking trail). Grass, flowers and dirt are walkable;
// trees, water and oil palms (Sawit) are solid. A tree that burns too long becomes
// Sawit and stays that way: it no longer counts as forest and cannot burn again.
enum class TileType : std::uint8_t { Grass, GrassFlowers, Tree, Water, Dirt, Sawit };

// Placeholder forest map: a fixed-seed random grid of grass, trees, a dirt trail and a pond.
// Tiles can change during play (tree -> sawit), and trees remember how scorched they are.
// (docs/05-implementation-plan.md).
class ForestMap {
public:
    ForestMap(int widthInTiles, int heightInTiles, unsigned seed);

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    TileType GetTile(int x, int y) const; // outside the map counts as Tree
    bool IsPath(int x, int y) const { return GetTile(x, y) == TileType::Dirt; }
    bool IsInside(int x, int y) const { return x >= 0 && y >= 0 && x < m_width && y < m_height; }
    bool IsBorder(int x, int y) const { return x == 0 || y == 0 || x == m_width - 1 || y == m_height - 1; }
    // Trees that can catch fire: every tree except the ring that forms the map edge.
    bool IsBurnable(int x, int y) const { return IsInside(x, y) && !IsBorder(x, y) && GetTile(x, y) == TileType::Tree; }

    // Changes one tile; ignored outside the map. Bumps GetRevision().
    void SetTile(int x, int y, TileType type);
    // Counts a tile type, optionally skipping the border ring (border trees are scenery).
    int CountTiles(TileType type, bool includeBorder = false) const;
    // Increases on every SetTile, so views such as the minimap know when to redraw.
    int GetRevision() const { return m_revision; }

    // 0 = untouched, 1 = fully charred. Burning trees get darker; an extinguished tree
    // keeps its scar.
    float GetScorch(int x, int y) const;
    void AddScorch(int x, int y, float amount);

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

    // Ground and shadows are always below the depth-sorted trees/player.
    void DrawGround(engine::SpriteBatch& batch, const engine::Texture& atlas,
              const engine::Rect& visibleArea, float time = 0.0f) const;
    // Draw rear trees before the player, then foreground trees afterwards.
    // Depth uses roots/feet in world coordinates, regardless of facing or X.
    // sawitTexture is drawn whole for Sawit tiles (roots at the bottom of the tile).
    void DrawTrees(engine::SpriteBatch& batch, const engine::Texture& texture,
                   const engine::Rect& source, const engine::Texture& sawitTexture,
                   const engine::Rect& visibleArea, float playerFeetY, bool foreground) const;

private:
    // Shape of the placeholder trail; used once while generating the map.
    bool IsOnTrail(int x, int y) const;

    int m_width = 0;
    int m_height = 0;
    std::vector<TileType> m_tiles; // row-major: index = y * width + x
    std::vector<float> m_scorch;   // same layout as m_tiles
    int m_revision = 0;
};

} // namespace game
