#include "entities/WaterDrop.h"

#include <engine/graphics/SpriteBatch.h>

#include <algorithm>

namespace game {

using engine::Color;

WaterDrop::WaterDrop(engine::Vec2 position, engine::Vec2 velocity, float lifetime)
    : engine::Entity(position), m_velocity(velocity), m_lifetime(lifetime) {}

void WaterDrop::Update(float dt) {
    m_position += m_velocity * dt;
    m_velocity *= std::max(0.0f, 1.0f - 1.6f * dt); // air drag: the stream spreads out at the end
    m_age += dt;
    if (m_age >= m_lifetime) {
        Destroy();
    }
}

void WaterDrop::Draw(engine::SpriteBatch& batch, float alpha) const {
    const engine::Vec2 p = engine::Round(GetRenderPosition(alpha));
    const float life = 1.0f - m_age / m_lifetime;
    const auto a = static_cast<std::uint8_t>(110 + 140 * life);
    batch.DrawRect({p.x - 1.0f, p.y - 1.0f, 2.0f, 2.0f}, Color::FromHex(0x5FB8E0, a));
    batch.DrawRect({p.x - 1.0f, p.y - 1.0f, 1.0f, 1.0f}, Color::FromHex(0xD8F3FF, a));
}

} // namespace game
