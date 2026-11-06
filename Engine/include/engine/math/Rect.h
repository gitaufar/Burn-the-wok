#pragma once

#include "engine/math/Vec2.h"

namespace engine {

// Axis-aligned rectangle. (x, y) is the top-left corner; y grows downward,
// matching screen and tile-map coordinates.
struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;

    constexpr float Left() const { return x; }
    constexpr float Top() const { return y; }
    constexpr float Right() const { return x + w; }
    constexpr float Bottom() const { return y + h; }
    constexpr Vec2 Position() const { return {x, y}; }
    constexpr Vec2 Size() const { return {w, h}; }
    constexpr Vec2 Center() const { return {x + w * 0.5f, y + h * 0.5f}; }

    // Half-open on the right/bottom edge so neighbouring tiles never both contain a point.
    constexpr bool Contains(Vec2 p) const {
        return p.x >= x && p.x < x + w && p.y >= y && p.y < y + h;
    }
};

} // namespace engine
