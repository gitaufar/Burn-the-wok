#include "engine/physics/Collision.h"

#include <algorithm>
#include <cmath>

namespace engine {

namespace {

// Positions accumulate float error (e.g. 47.99999 instead of 48). Shrinking edges by
// this much before converting to tile indices keeps a box that is flush against a
// tile from being reported as overlapping it.
constexpr float Epsilon = 1e-3f;

// First tile index covered by a span starting at 'start'.
int FirstTile(float start, float tileSize) {
    return static_cast<int>(std::floor((start + Epsilon) / tileSize));
}

// Last tile index covered by a span ending at 'end' (exclusive edge).
int LastTile(float end, float tileSize) {
    return static_cast<int>(std::ceil((end - Epsilon) / tileSize)) - 1;
}

// Moves a span [pos, pos + size) by delta along one axis. crossFirst..crossLast are the
// tiles the box covers on the other axis. isSolidAt(along, across) is the tile test with
// the axes already mapped. Returns true when a solid tile stopped the move.
template <typename SolidAt>
bool SweepAxis(float& pos, float size, float delta, int crossFirst, int crossLast,
               float tileSize, const SolidAt& isSolidAt) {
    if (delta == 0.0f) {
        return false;
    }

    auto lineIsSolid = [&](int line) {
        for (int cross = crossFirst; cross <= crossLast; ++cross) {
            if (isSolidAt(line, cross)) {
                return true;
            }
        }
        return false;
    };

    if (delta > 0.0f) {
        // Only lines the leading edge newly enters are checked, which is what lets a
        // box that already overlaps a solid tile walk out of it.
        const int from = LastTile(pos + size, tileSize) + 1;
        const int to = LastTile(pos + size + delta, tileSize);
        for (int line = from; line <= to; ++line) {
            if (lineIsSolid(line)) {
                pos = line * tileSize - size;
                return true;
            }
        }
    } else {
        const int from = FirstTile(pos, tileSize) - 1;
        const int to = FirstTile(pos + delta, tileSize);
        for (int line = from; line >= to; --line) {
            if (lineIsSolid(line)) {
                pos = (line + 1) * tileSize;
                return true;
            }
        }
    }

    pos += delta;
    return false;
}

} // namespace

bool Overlaps(const Rect& a, const Rect& b) {
    return a.Left() < b.Right() && b.Left() < a.Right() &&
           a.Top() < b.Bottom() && b.Top() < a.Bottom();
}

bool Overlaps(const Circle& a, const Circle& b) {
    const float radii = a.radius + b.radius;
    return (a.center - b.center).LengthSquared() < radii * radii;
}

Vec2 ClosestPoint(const Rect& rect, Vec2 point) {
    return {std::clamp(point.x, rect.Left(), rect.Right()),
            std::clamp(point.y, rect.Top(), rect.Bottom())};
}

bool Overlaps(const Circle& circle, const Rect& rect) {
    const Vec2 nearest = ClosestPoint(rect, circle.center);
    return (circle.center - nearest).LengthSquared() < circle.radius * circle.radius;
}

bool Contains(const Circle& circle, Vec2 point) {
    return (point - circle.center).LengthSquared() < circle.radius * circle.radius;
}

TileRange TilesOverlapping(const Rect& area, float tileSize) {
    if (area.w <= 0.0f || area.h <= 0.0f || tileSize <= 0.0f) {
        return {};
    }
    return {FirstTile(area.Left(), tileSize), FirstTile(area.Top(), tileSize),
            LastTile(area.Right(), tileSize), LastTile(area.Bottom(), tileSize)};
}

bool OverlapsSolidTile(const Rect& area, float tileSize, const TileSolidFn& isSolid) {
    const TileRange tiles = TilesOverlapping(area, tileSize);
    for (int y = tiles.minY; y <= tiles.maxY; ++y) {
        for (int x = tiles.minX; x <= tiles.maxX; ++x) {
            if (isSolid(x, y)) {
                return true;
            }
        }
    }
    return false;
}

MoveResult MoveAndSlide(const Rect& box, Vec2 delta, float tileSize, const TileSolidFn& isSolid) {
    MoveResult result{box};
    Rect& b = result.box;

    // Horizontal first, using the rows the box covers before moving...
    result.hitX = SweepAxis(b.x, b.w, delta.x, FirstTile(b.Top(), tileSize),
                            LastTile(b.Bottom(), tileSize), tileSize,
                            [&](int column, int row) { return isSolid(column, row); });

    // ...then vertical, using the columns it covers after the horizontal step.
    result.hitY = SweepAxis(b.y, b.h, delta.y, FirstTile(b.Left(), tileSize),
                            LastTile(b.Right(), tileSize), tileSize,
                            [&](int row, int column) { return isSolid(column, row); });
    return result;
}

} // namespace engine
