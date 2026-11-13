#include "entities/Particle.h"

#include <engine/graphics/SpriteBatch.h>

#include <cmath>

namespace game {

Particle::Particle(engine::Vec2 position, const Settings& settings)
    : engine::Entity(position), m_settings(settings) {}

void Particle::Update(float dt) {
    m_position += m_settings.velocity * dt;
    m_age += dt;
    if (m_age >= m_settings.lifetime) {
        Destroy();
    }
}

void Particle::Draw(engine::SpriteBatch& batch, float alpha) const {
    const float t = m_age / m_settings.lifetime;
    const float size = std::round(m_settings.startSize + (m_settings.endSize - m_settings.startSize) * t);
    const engine::Vec2 p = engine::Round(GetRenderPosition(alpha));
    const auto a = static_cast<std::uint8_t>(m_settings.color.a * (1.0f - t));
    batch.DrawRect({p.x - std::floor(size * 0.5f), p.y - std::floor(size * 0.5f), size, size},
                   m_settings.color.WithAlpha(a));
}

} // namespace game
