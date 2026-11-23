#include "entities/Volunteer.h"

#include "entities/Fire.h"
#include "entities/StageActor.h"
#include "world/ForestMap.h"
#include <engine/entity/EntityWorld.h>
#include <engine/graphics/SpriteBatch.h>
#include <algorithm>
#include <cmath>
#include <limits>

namespace game {
using namespace navigation;
using engine::Vec2;
using engine::Color;

Volunteer::Volunteer(Vec2 start, unsigned seed, float duty)
    : Entity(start), m_sprayer(seed, config::DropPower * 0.6f), m_duty(duty), m_temporary(duty > 0) {}

bool Volunteer::IsExit(const ForestMap& map, Cell cell) {
    return cell.x <= 1 || cell.y <= 1 || cell.x >= map.GetWidth()-2 || cell.y >= map.GetHeight()-2;
}

void Volunteer::Walk(float dt, float speedScale) {
    if (m_route.empty()) return;
    const auto delta = CellCenter(m_route.back()) - m_position;
    const float step = 67.0f * dt * speedScale;
    if (delta.LengthSquared() <= step * step) {
        m_position = CellCenter(m_route.back());
        m_route.pop_back();
    } else {
        m_facing = delta.Normalized();
        m_position += m_facing * step;
        m_stride += dt;
    }
}

void Volunteer::Step(float dt, const ForestMap& map, engine::EntityWorld& world) {
    const Cell here = ToCell(m_position);
    float speedScale = 1;
    static_cast<const engine::EntityWorld&>(world).ForEach<StageActor>([&](const StageActor& actor) {
        if (actor.BlocksRanger() && (actor.GetPosition() - m_position).LengthSquared() < config::BossBlockDistance * config::BossBlockDistance)
            speedScale = config::BossSlowMultiplier;
    });
    if (m_temporary) {
        if (m_duty > 0) {
            m_duty -= dt;
            if (m_duty <= 0) m_replan = 0;
        }
        if (m_duty <= 0) {
            if (IsExit(map, here)) { Destroy(); return; }
            m_replan -= dt;
            if (m_replan <= 0 || m_mapRevision != map.GetRevision()) {
                m_mapRevision = map.GetRevision();
                m_route = FindRoute(map, here, [&](Cell cell) { return IsExit(map, cell); });
                m_replan = 0.7f;
            }
            Walk(dt, speedScale);
            return;
        }
    }
    auto pond = [&](Cell cell) {
        for (const auto offset : Neighbors)
            if (map.GetTile(cell.x+offset.x, cell.y+offset.y) == TileType::Water) return true;
        return false;
    };
    if (m_tank.GetAmount() < config::WaterPerDrop) m_refill = true;
    if (m_refill && pond(here)) {
        m_tank.Refill(config::RefillPerSecond * dt);
        if (m_tank.IsFull()) { m_refill = false; m_replan = 0; }
        return;
    }
    Vec2 target;
    float nearest = std::numeric_limits<float>::max();
    const auto& entities = static_cast<const engine::EntityWorld&>(world);
    entities.ForEach<Fire>([&](const Fire& fire) {
        if (fire.IsExtinguished()) return;
        const auto point = fire.GetHitArea().center;
        const float distance = (point-m_position).LengthSquared();
        if (distance < nearest) { nearest = distance; target = point; }
    });
    if (!m_refill && nearest < 45*45) {
        m_facing = (target - m_position).Normalized();
        m_sprayer.Update(dt, true, m_position, m_facing, m_tank, world);
        m_replan = 0;
        return;
    }
    m_sprayer.Update(dt, false, {}, {}, m_tank, world);
    m_replan -= dt;
    if (m_replan <= 0 || m_mapRevision != map.GetRevision()) {
        m_mapRevision = map.GetRevision();
        const bool fireFound = nearest < std::numeric_limits<float>::max();
        m_route.clear();
        if (m_refill) {
            m_route = FindRoute(map, here, pond);
        } else if (fireFound) {
            m_route = FindRoute(map, here, [&](Cell cell) {
                return (CellCenter(cell) - target).LengthSquared() < 42*42;
            });
            if (m_route.empty()) {
                m_route = FindRoute(map, here, [&](Cell cell) {
                    bool inRange = false;
                    entities.ForEach<Fire>([&](const Fire& fire) {
                        if (!fire.IsExtinguished())
                            inRange |= (CellCenter(cell) - fire.GetHitArea().center).LengthSquared() < 42*42;
                    });
                    return inRange;
                });
            }
        }
        m_replan = 0.7f;
    }
    Walk(dt, speedScale);
}

void Volunteer::Draw(engine::SpriteBatch& batch, float alpha) const {
    const auto feet = GetRenderPosition(alpha);
    const auto px = [&](float x, float y, float w, float h, unsigned shade) {
        batch.DrawRect({std::round(feet.x+x), std::round(feet.y+y), w, h}, Color::FromHex(shade));
    };
    const int gait = m_route.empty() ? 0 : static_cast<int>(m_stride * 12) % 4;
    const bool side = std::abs(m_facing.x) > std::abs(m_facing.y);
    const float direction = m_facing.x < 0 ? -1.0f : 1.0f;
    px(-6, 0, 12, 2, 0x253D30);
    px(-4, -7, 3, 7.0f-(gait == 1 ? 1.0f : 0.0f), 0x253749);
    px(1, -7, 3, 7.0f-(gait == 3 ? 1.0f : 0.0f), 0x253749);
    px(-6, -14, 12, 8, 0x273F3D);
    px(-4, -14, 8, 8, m_temporary ? 0xC77448 : 0x468C88);
    px(-4, -9, 8, 1, 0xF0D998);
    px(-3, -19, 6, 5, m_facing.y < -0.7f ? 0x584537 : 0xC58F67);
    px(-5, -21, 10, 3, 0xE0AD54);
    px(-3, -23, 6, 2, 0xE0AD54);
    px(side ? direction*4-1 : -5, -18, side ? 4.0f : 10.0f, 1, 0xAD7937);
    if (m_sprayer.IsSpraying()) px(side ? direction*7-2 : -2, -11, 4, 2, 0x9ED5DB);
}
}
