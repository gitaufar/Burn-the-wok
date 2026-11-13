#pragma once

#include <engine/entity/Entity.h>
#include <engine/physics/Collision.h>

namespace game {

// One drop of the water spray. Flies straight, slows down a little, and evaporates
// after a fixed lifetime, which limits the sprayer's range. Drops fly over trees on
// purpose: the fire sits on a tree, so the water has to reach it.
class WaterDrop : public engine::Entity {
public:
    WaterDrop(engine::Vec2 position, engine::Vec2 velocity, float lifetime);

    void Update(float dt) override;
    void Draw(engine::SpriteBatch& batch, float alpha) const override;

    engine::Circle GetHitArea() const { return {m_position, 2.0f}; }

private:
    engine::Vec2 m_velocity;
    float m_age = 0.0f;
    float m_lifetime;
};

} // namespace game
