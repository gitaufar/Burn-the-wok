#include "world/Navigation.h"
#include "world/ForestMap.h"
#include "GameConfig.h"

#include <cmath>

namespace game::navigation {
using engine::Vec2;
constexpr float Tile = config::TileSize;
Cell ToCell(Vec2 position) {
    return {static_cast<int>(std::floor(position.x / Tile)), static_cast<int>(std::floor(position.y / Tile))};
}

Vec2 CellCenter(Cell cell) {
    return {(cell.x + 0.5f) * Tile, (cell.y + 0.5f) * Tile};
}

std::vector<Cell> FindRoute(const ForestMap& map, Cell start,
                                         const std::function<bool(Cell)>& goal) {
    if (map.IsSolid(start.x, start.y)) return {};
    const int width = map.GetWidth();
    const auto index = [width](Cell cell) { return cell.y * width + cell.x; };
    std::vector<int> parent(width * map.GetHeight(), -1);
    std::vector<Cell> queue{start};
    parent[index(start)] = index(start);

    for (std::size_t i = 0; i < queue.size(); ++i) {
        const Cell cell = queue[i];
        if (goal(cell)) {
            std::vector<Cell> route;
            for (int at = index(cell); at != index(start); at = parent[at]) {
                route.push_back({at % width, at / width});
            }
            route.push_back(start);
            return route;
        }
        for (const Cell offset : Neighbors) {
            const Cell next{cell.x + offset.x, cell.y + offset.y};
            if (map.IsSolid(next.x, next.y) || parent[index(next)] != -1) continue;
            parent[index(next)] = index(cell);
            queue.push_back(next);
        }
    }
    return {};
}

}
