#pragma once

#include <engine/math/Vec2.h>

#include <random>

namespace engine {
class EntityWorld;
}

namespace game {

class WaterTank;

// Turns "trigger held" into a stream of WaterDrop entities. Drops come out at a fixed
// rate (not once per frame), each costs a fixed amount of tank water, and each gets a
// small random angle so the stream looks like a spray instead of a laser.
class Sprayer {
public:
    explicit Sprayer(unsigned seed) : m_rng(seed) {}

    // nozzle = world point the water leaves from; aim = normalized direction.
    void Update(float dt, bool triggerHeld, engine::Vec2 nozzle, engine::Vec2 aim,
                WaterTank& tank, engine::EntityWorld& world);

    // True while water is actually coming out (trigger held AND the tank had water).
    bool IsSpraying() const { return m_spraying; }

private:
    std::mt19937 m_rng;
    float m_emitTimer = 0.0f; // seconds until the next drop
    bool m_spraying = false;
};

} // namespace game
