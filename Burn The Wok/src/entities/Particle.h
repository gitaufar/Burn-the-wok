#pragma once

#include <engine/entity/Entity.h>
#include <engine/graphics/Color.h>

namespace game {

// Short-lived square puff (smoke from a put-out fire, steam where water hits flames).
// Drifts with a constant velocity, grows, and fades out over its lifetime.
class Particle : public engine::Entity {
public:
    struct Settings {
        engine::Vec2 velocity;
        float lifetime = 1.0f;
        float startSize = 2.0f;
        float endSize = 4.0f;
        engine::Color color = engine::Color::White();
    };

    Particle(engine::Vec2 position, const Settings& settings);

    void Update(float dt) override;
    void Draw(engine::SpriteBatch& batch, float alpha) const override;

private:
    Settings m_settings;
    float m_age = 0.0f;
};

} // namespace game
