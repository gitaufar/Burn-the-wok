#include "systems/Sprayer.h"

#include "GameConfig.h"
#include "entities/WaterDrop.h"
#include "systems/WaterTank.h"

#include <engine/entity/EntityWorld.h>

#include <cmath>

namespace game {

void Sprayer::Update(float dt, bool triggerHeld, engine::Vec2 nozzle, engine::Vec2 aim,
                     WaterTank& tank, engine::EntityWorld& world) {
    m_spraying = false;
    if (!triggerHeld || aim == engine::Vec2{}) {
        // Releasing the trigger resets the timer, so the first drop of the next
        // press comes out immediately.
        m_emitTimer = 0.0f;
        return;
    }

    constexpr float Pi = 3.14159265f;
    const float interval = 1.0f / config::SprayDropsPerSecond;
    std::uniform_real_distribution<float> spread(-config::SpraySpreadDegrees, config::SpraySpreadDegrees);
    std::uniform_real_distribution<float> speedJitter(0.9f, 1.1f);

    m_emitTimer -= dt;
    while (m_emitTimer <= 0.0f) {
        m_emitTimer += interval;
        if (!tank.TryUse(config::WaterPerDrop)) {
            m_emitTimer = 0.0f;
            return;
        }
        m_spraying = true;
        const float angle = std::atan2(aim.y, aim.x) + spread(m_rng) * Pi / 180.0f;
        const engine::Vec2 direction{std::cos(angle), std::sin(angle)};
        world.Spawn<WaterDrop>(nozzle, direction * config::DropSpeed * speedJitter(m_rng),
                               config::DropLifetime);
    }
    // Between drops (timer still positive) water is still flowing.
    m_spraying = m_spraying || tank.GetAmount() >= config::WaterPerDrop;
}

} // namespace game
