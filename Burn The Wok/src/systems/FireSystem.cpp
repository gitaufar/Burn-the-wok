#include "systems/FireSystem.h"

#include "GameConfig.h"
#include "entities/Fire.h"
#include "entities/Particle.h"
#include "entities/WaterDrop.h"
#include "world/ForestMap.h"

#include <engine/entity/EntityWorld.h>

#include <algorithm>
#include <utility>
#include <vector>

namespace game {

using engine::Color;
using engine::Vec2;

int FireSystem::IgniteRandomTrees(engine::EntityWorld& world, const ForestMap& map, int count) {
    std::vector<std::pair<int, int>> candidates;
    for (int y = 1; y < map.GetHeight() - 1; ++y) {
        for (int x = 1; x < map.GetWidth() - 1; ++x) {
            if (!map.IsBurnable(x, y) || IsBurning(world, x, y)) continue;
            const bool reachable = !map.IsSolid(x + 1, y) || !map.IsSolid(x - 1, y) ||
                                   !map.IsSolid(x, y + 1) || !map.IsSolid(x, y - 1);
            if (reachable) candidates.emplace_back(x, y);
        }
    }
    std::shuffle(candidates.begin(), candidates.end(), m_rng);

    int lit = 0;
    for (const auto& [x, y] : candidates) {
        if (lit == count) break;
        if (Ignite(world, map, x, y)) ++lit;
    }
    return lit;
}

bool FireSystem::Ignite(engine::EntityWorld& world, const ForestMap& map, int tileX, int tileY) {
    if (!map.IsBurnable(tileX, tileY) || IsBurning(world, tileX, tileY)) return false;
    world.Spawn<Fire>(tileX, tileY, *m_atlas);
    return true;
}

bool FireSystem::IsBurning(const engine::EntityWorld& world, int tileX, int tileY) const {
    bool burning = false;
    world.ForEach<Fire>([&](const Fire& fire) {
        burning = burning || (fire.GetTileX() == tileX && fire.GetTileY() == tileY);
    });
    return burning;
}

FireSystem::StepResult FireSystem::Update(engine::EntityWorld& world, ForestMap& map, float dt) {
    StepResult result;
    std::uniform_real_distribution<float> unit(-1.0f, 1.0f);
    // Ignitions are collected first and applied after the loop: a fire spawned inside
    // ForEach only joins the world afterwards, so IsBurning() could not see it yet and
    // two sparks might pick the same tree in one step.
    std::vector<std::pair<int, int>> sparks;

    world.ForEach<Fire>([&](Fire& fire) {
        const int x = fire.GetTileX(), y = fire.GetTileY();
        map.AddScorch(x, y, dt / config::BurnToSawitSeconds);

        if (fire.GetBurnTime() >= config::BurnToSawitSeconds) {
            // The burnt tree is cleared and planted with oil palm.
            map.SetTile(x, y, TileType::Sawit);
            fire.Destroy();
            ++result.turnedToSawit;
            const Vec2 center = fire.GetHitArea().center;
            for (int i = 0; i < 8; ++i) {
                world.Spawn<Particle>(center + Vec2{unit(m_rng) * 6.0f, unit(m_rng) * 3.0f},
                    Particle::Settings{{unit(m_rng) * 4.0f, -10.0f - 5.0f * (unit(m_rng) + 1.0f)},
                                       1.4f + 0.3f * unit(m_rng), 2.0f, 7.0f, Color::FromHex(0x4A4440, 210)});
            }
            return;
        }

        if (fire.IsReadyToSpread()) {
            fire.RestartSpreadTimer();
            std::vector<std::pair<int, int>> candidates;
            const int r = config::SpreadRadiusTiles;
            for (int ty = y - r; ty <= y + r; ++ty) {
                for (int tx = x - r; tx <= x + r; ++tx) {
                    if ((tx != x || ty != y) && map.IsBurnable(tx, ty) && !IsBurning(world, tx, ty)) {
                        candidates.emplace_back(tx, ty);
                    }
                }
            }
            if (!candidates.empty()) {
                std::uniform_int_distribution<std::size_t> pick(0, candidates.size() - 1);
                sparks.push_back(candidates[pick(m_rng)]);
            }
        }
    });

    for (const auto& [x, y] : sparks) {
        if (Ignite(world, map, x, y)) ++result.spread;
    }
    return result;
}

int FireSystem::ResolveWaterHits(engine::EntityWorld& world) {
    std::uniform_real_distribution<float> unit(-1.0f, 1.0f);
    int extinguished = 0;

    world.ForEach<WaterDrop>([&](WaterDrop& drop) {
        world.ForEach<Fire>([&](Fire& fire) {
            if (!drop.IsAlive() || !fire.IsAlive()) return;
            if (!engine::Overlaps(drop.GetHitArea(), fire.GetHitArea())) return;

            drop.Destroy();
            fire.ApplyWater(config::DropPower);
            // A little steam where the water lands.
            if (unit(m_rng) > 0.3f) {
                world.Spawn<Particle>(drop.GetPosition(), Particle::Settings{
                    {unit(m_rng) * 6.0f, -14.0f}, 0.45f, 1.0f, 3.0f, Color::FromHex(0xE8F1F2, 190)});
            }
            if (fire.IsExtinguished()) {
                fire.Destroy();
                ++extinguished;
                const Vec2 center = fire.GetHitArea().center;
                for (int i = 0; i < 10; ++i) {
                    world.Spawn<Particle>(center + Vec2{unit(m_rng) * 5.0f, unit(m_rng) * 3.0f},
                        Particle::Settings{{unit(m_rng) * 5.0f, -12.0f - 6.0f * (unit(m_rng) + 1.0f)},
                                           1.1f + 0.3f * unit(m_rng), 2.0f, 6.0f,
                                           Color::FromHex(i % 2 ? 0x6E6A64 : 0x9A958C, 200)});
                }
            }
        });
    });
    return extinguished;
}

} // namespace game
