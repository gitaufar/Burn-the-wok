#include "entities/StageActor.h"

#include "systems/FireSystem.h"
#include "world/ForestMap.h"

#include <engine/entity/EntityWorld.h>
#include <engine/graphics/Image.h>
#include <engine/graphics/SpriteBatch.h>

#include <algorithm>
#include <cmath>
#include <string>

namespace game {

using engine::Vec2;
using engine::Color;

namespace {
using Cell = StageActor::Cell;
constexpr float Tile = config::TileSize;
constexpr Cell Neighbors[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
// Reuse the Wowo set for the saboteur until the team has dedicated artwork.
constexpr const char* ArtFolders[] = {"orangutan", "bekantan", "burung_hantu", "wowo", "wowo", "tedduy"};
}

StageActor::StageActor(Kind kind, Vec2 position)
    : Entity(position), m_kind(kind), m_actionTimer(kind == Kind::Saboteur ? 5.0f : 0.0f) {}

void StageActor::LoadArtwork() {
    constexpr const char* Directions[] = {"front", "back", "left", "right"};
    const std::string folder = std::string("assets/characters/") + ArtFolders[static_cast<int>(m_kind)] + "/";
    float tallest = 1.0f;
    for (int i = 0; i < static_cast<int>(m_frames.size()); ++i) {
        const auto image = engine::Image::LoadFromFile(folder + Directions[i / 4] + "_" +
                                                      std::to_string(i % 4 + 1) + ".png");
        m_sources[i] = image.GetContentBounds(16);
        tallest = std::max(tallest, m_sources[i].h);
        m_frames[i] = std::make_unique<engine::Texture>(image);
    }
    m_artScale = (IsAnimal() ? 20.0f : config::PlayerHeight) / tallest;
}

void StageActor::Draw(engine::SpriteBatch& batch, float alpha) const {
    if (m_rescue == Rescue::Saved || m_rescue == Rescue::Burned) return;
    const int frame = m_facing * 4 + static_cast<int>(m_walkTime * 10.0f) % 4;
    if (!m_frames[frame]) return;
    const auto source = m_sources[frame];
    const Vec2 feet = engine::Round(GetRenderPosition(alpha));
    const float w = source.w * m_artScale, h = source.h * m_artScale;
    const Color tint = m_rescue == Rescue::Panicked ? Color::FromHex(0xFFC88F) : Color::White();
    batch.Draw(*m_frames[frame], {feet.x - w * 0.5f, feet.y - h + 2, w, h}, source, tint);
}

Cell StageActor::ToCell(Vec2 position) {
    return {static_cast<int>(std::floor(position.x / Tile)), static_cast<int>(std::floor(position.y / Tile))};
}

Vec2 StageActor::CellCenter(Cell cell) {
    return {(cell.x + 0.5f) * Tile, (cell.y + 0.5f) * Tile};
}

std::vector<Cell> StageActor::FindRoute(const ForestMap& map, Cell start,
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

engine::Rect StageActor::GetFeet() const {
    return {m_position.x - 4, m_position.y - 3, 8, 6};
}

bool StageActor::MoveToward(Vec2 target, float speed, float dt, const ForestMap& map) {
    const Vec2 delta = target - m_position;
    const Vec2 step = delta.Normalized() * std::min(delta.Length(), speed * dt);
    const auto feet = GetFeet();
    const auto moved = engine::MoveAndSlide(feet, step, Tile,
        [&map](int x, int y) { return map.IsSolid(x, y); });
    const Vec2 movement = moved.box.Position() - feet.Position();
    m_position += movement;
    if (movement.LengthSquared() > 0.0001f) {
        if (std::abs(movement.x) > std::abs(movement.y)) m_facing = movement.x < 0 ? 2 : 3;
        else m_facing = movement.y < 0 ? 1 : 0;
        m_walkTime = std::fmod(m_walkTime + dt, 0.4f);
    }
    return (target - m_position).LengthSquared() < 0.001f;
}

void StageActor::FollowRoute(float speed, float dt, const ForestMap& map) {
    if (!m_route.empty() && MoveToward(CellCenter(m_route.back()), speed, dt, map)) {
        m_route.pop_back();
    }
}

void StageActor::StepAnimal(float dt, const ForestMap& map, const engine::EntityWorld& world, Vec2 ranger) {
    if (m_rescue == Rescue::Saved || m_rescue == Rescue::Burned) return;
    float nearestFire = 80.0f;
    world.ForEach<Fire>([&](const Fire& fire) {
        nearestFire = std::min(nearestFire, (fire.GetHitArea().center - m_position).Length());
    });
    m_exposure = nearestFire < 19.0f ? m_exposure + dt : 0.0f;
    if (m_exposure >= 2.0f) {
        m_rescue = Rescue::Burned;
        return;
    }
    if (m_rescue == Rescue::Waiting && ((ranger - m_position).Length() < 28.0f || nearestFire < 30.0f)) {
        m_rescue = Rescue::Panicked;
    }
    if (m_rescue == Rescue::Panicked) {
        m_panicTime += dt;
        if (m_panicTime < 0.45f) return;
        m_rescue = Rescue::Fleeing;
    }
    if (m_rescue != Rescue::Fleeing) return;

    const Cell shelter = ToCell(map.GetCenter());
    if (m_routeRevision != map.GetRevision()) {
        m_route = FindRoute(map, ToCell(m_position), [&](Cell cell) { return cell == shelter; });
        m_routeRevision = map.GetRevision();
    }
    FollowRoute(46.0f, dt, map);
    if ((CellCenter(shelter) - m_position).LengthSquared() < 0.001f) m_rescue = Rescue::Saved;
}

void StageActor::StepSaboteur(float dt, ForestMap& map, engine::EntityWorld& world, FireSystem& fires,
                               const engine::Rect& rangerFeet) {
    m_planting = false;
    m_actionTimer -= dt;
    if (m_actionTimer > 0.0f) return;
    const auto findAction = [&](Cell from, TileType type, Cell& target) {
        for (const Cell offset : Neighbors) {
            const Cell next{from.x + offset.x, from.y + offset.y};
            if (map.GetTile(next.x, next.y) != type) continue;
            if (type == TileType::Tree && (!map.IsBurnable(next.x, next.y) || fires.IsBurning(world, next.x, next.y))) continue;
            target = next;
            return true;
        }
        return false;
    };

    if (m_route.empty() || m_routeRevision != map.GetRevision()) {
        Cell target{};
        const Cell start = ToCell(m_position);
        m_route = FindRoute(map, start, [&](Cell cell) { return findAction(cell, TileType::Charred, target); });
        if (m_route.empty() && m_ignitionsLeft > 0) {
            m_route = FindRoute(map, start, [&](Cell cell) { return findAction(cell, TileType::Tree, target); });
        }
        m_routeRevision = map.GetRevision();
        if (m_route.empty()) {
            m_actionTimer = 1.0f;
            return;
        }
    }
    FollowRoute(48.0f, dt, map);
    if (!m_route.empty()) return;

    Cell target{};
    const Cell from = ToCell(m_position);
    if (findAction(from, TileType::Charred, target)) {
        const auto tile = map.GetTileRect(target.x, target.y);
        bool occupied = engine::Overlaps(rangerFeet, tile);
        static_cast<const engine::EntityWorld&>(world).ForEach<StageActor>([&](const StageActor& actor) {
            if (actor.GetRescue() != Rescue::Saved && actor.GetRescue() != Rescue::Burned) {
                occupied |= engine::Overlaps(actor.GetFeet(), tile);
            }
        });
        if (!occupied) {
            map.SetTile(target.x, target.y, TileType::Sawit);
            m_planting = true;
        }
        m_actionTimer = occupied ? 1.0f : 4.0f;
    } else if (m_ignitionsLeft > 0 && findAction(from, TileType::Tree, target) &&
               fires.Ignite(world, map, target.x, target.y)) {
        --m_ignitionsLeft;
        m_actionTimer = 8.0f;
    }
}

void StageActor::StepBoss(float dt, const ForestMap& map, Vec2 ranger) {
    const float distance = (ranger - m_position).Length();
    if (distance > 140.0f || distance < 12.0f) return;
    m_actionTimer -= dt;
    if (m_actionTimer <= 0.0f || m_routeRevision != map.GetRevision()) {
        const Cell target = ToCell(ranger);
        m_route = FindRoute(map, ToCell(m_position), [&](Cell cell) { return cell == target; });
        m_routeRevision = map.GetRevision();
        m_actionTimer = 0.65f;
    }
    if (m_route.empty()) MoveToward(ranger, 45.0f, dt, map);
    else FollowRoute(45.0f, dt, map);
}

void StageActor::Step(float dt, ForestMap& map, engine::EntityWorld& world, FireSystem& fires,
                       Vec2 ranger, const engine::Rect& rangerFeet) {
    if (IsAnimal()) StepAnimal(dt, map, world, ranger);
    else if (m_kind == Kind::Saboteur) StepSaboteur(dt, map, world, fires, rangerFeet);
    else StepBoss(dt, map, ranger);
}

} // namespace game
