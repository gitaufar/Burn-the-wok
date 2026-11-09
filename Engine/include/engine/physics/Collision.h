#pragma once

#include "engine/math/Rect.h"
#include "engine/math/Vec2.h"

#include <functional>

namespace engine {

struct Circle {
    Vec2 center;
    float radius = 0.0f;
};

// Inclusive range of tile coordinates. Empty when max < min on either axis.
struct TileRange {
    int minX = 0;
    int minY = 0;
    int maxX = -1;
    int maxY = -1;

    constexpr bool IsEmpty() const { return maxX < minX || maxY < minY; }
};

// --- Shape overlap tests ------------------------------------------------------
// Shapes that only touch along an edge do NOT overlap. This matches Rect::Contains
// (half-open) and lets a box rest flush against a wall without counting as a hit.

bool Overlaps(const Rect& a, const Rect& b);
bool Overlaps(const Circle& a, const Circle& b);
bool Overlaps(const Circle& circle, const Rect& rect);
bool Contains(const Circle& circle, Vec2 point);

// Point of rect nearest to point (point itself when it lies inside rect).
Vec2 ClosestPoint(const Rect& rect, Vec2 point);

// --- Tile grid queries ----------------------------------------------------------
// The engine does not know what a tile *is*; the game answers "is tile (x, y) solid?"
// through this callback. Tiles are squares of tileSize world units, tile (0, 0) has its
// top-left corner at the world origin.
using TileSolidFn = std::function<bool(int tileX, int tileY)>;

// Every tile that area overlaps (edge contact excluded).
TileRange TilesOverlapping(const Rect& area, float tileSize);

// True when any tile overlapped by area is solid.
bool OverlapsSolidTile(const Rect& area, float tileSize, const TileSolidFn& isSolid);

struct MoveResult {
    Rect box;          // box after the move
    bool hitX = false; // horizontal movement was stopped by a solid tile
    bool hitY = false; // vertical movement was stopped by a solid tile
};

// Moves box by delta through a tile grid, stopping flush against solid tiles.
// X and Y are resolved separately, so a box pushed diagonally into a wall slides
// along it instead of sticking. Every tile column/row between the start and the end
// is checked, so fast movers cannot tunnel through a thin wall. A box that starts
// inside a solid tile is still allowed to move out of it.
MoveResult MoveAndSlide(const Rect& box, Vec2 delta, float tileSize, const TileSolidFn& isSolid);

} // namespace engine
