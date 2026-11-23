#pragma once
#include <engine/math/Vec2.h>
#include <functional>
#include <vector>

namespace game {
class ForestMap;

namespace navigation {
struct Cell {
    int x;
    int y;
    bool operator==(const Cell&) const = default;
};
inline constexpr Cell Neighbors[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
Cell ToCell(engine::Vec2 position);
engine::Vec2 CellCenter(Cell cell);
std::vector<Cell> FindRoute(const ForestMap& map, Cell start,
                            const std::function<bool(Cell)>& goal);
}
}
