#include "entities/Fire.h"

#include "GameConfig.h"
#include "art/PlaceholderArt.h"

#include <engine/graphics/SpriteBatch.h>

#include <algorithm>
#include <cmath>

namespace game {

using engine::Color;
using engine::Vec2;

Fire::Fire(int tileX, int tileY, const FireArt& art)
    // Position = bottom-center of the tile, i.e. where the tree's roots are.
    : engine::Entity({(tileX + 0.5f) * config::TileSize, (tileY + 1.0f) * config::TileSize}),
      m_art(art),
      m_tileX(tileX),
      m_tileY(tileY),
      m_intensity(config::FireStartIntensity),
      m_spreadTimer(config::SpreadInterval),
      m_time(static_cast<float>((tileX * 7 + tileY * 13) % 10) * 0.1f) {}

void Fire::Update(float dt) {
    m_time += dt;
    m_sinceWater += dt;
    if (m_sinceWater >= config::FireRegrowDelay) {
        m_intensity = std::min(config::FireMaxIntensity, m_intensity + config::FireGrowthPerSecond * dt);
    }
    const float strength = GetIntensityFraction();
    m_burnTime += dt * (0.5f + 0.5f * strength);
    if (strength >= config::SpreadMinIntensityFraction) {
        m_spreadTimer -= dt;
    }
}

void Fire::RestartSpreadTimer() {
    m_spreadTimer = config::SpreadInterval;
}

void Fire::ApplyWater(float amount) {
    m_intensity = std::max(0.0f, m_intensity - amount);
    m_sinceWater = 0.0f;
}

float Fire::GetIntensityFraction() const {
    return m_intensity / config::FireMaxIntensity;
}

engine::Circle Fire::GetHitArea() const {
    // Centered on the flames (drawn over the trunk and lower crown), not on the roots.
    return {{m_position.x, m_position.y - 9.0f}, 6.0f + 5.0f * GetIntensityFraction()};
}

void Fire::Draw(engine::SpriteBatch& batch, float alpha) const {
    const Vec2 base = engine::Round(GetRenderPosition(alpha));
    const float strength = GetIntensityFraction();
    const float flicker = std::sin(m_time * 11.0f) * 0.5f + 0.5f;

    // Warm light on the ground and the tree, pulsing slightly.
    const float glowSize = 26.0f + 18.0f * strength + flicker * 3.0f;
    batch.Draw(*m_art.atlas, {base.x - glowSize * 0.5f, base.y - 9.0f - glowSize * 0.5f, glowSize, glowSize},
               GetSpriteSource(SpriteId::Glow),
               Color::FromHex(0xFF8A3D, static_cast<std::uint8_t>(70 + 60 * strength)));

    // Freshly hit flames turn pale (steam), which tells the player the spray is landing.
    const bool steaming = m_sinceWater < 0.12f;
    const Color tint = steaming ? Color::FromHex(0xFFE9D6) : Color::White();

    // One flame image, animated by mirroring it and stretching its height a little:
    // enough to read as flickering without hand-drawn frames.
    const engine::Rect src = m_art.flameSource;
    const engine::Rect mirrored{src.x + src.w, src.y, -src.w, src.h};
    const bool flip = static_cast<int>(m_time * 7.0f) % 2 == 1;
    auto flame = [&](float centerX, float bottomY, float height, bool mirror) {
        const float h = std::round(height * (1.0f + 0.07f * std::sin(m_time * 13.0f + centerX)));
        const float w = std::round(height * src.w / src.h);
        batch.Draw(*m_art.flame, {std::round(centerX - w * 0.5f), bottomY - h, w, h},
                   mirror ? mirrored : src, tint);
    };
    const float height = 12.0f + 14.0f * strength;
    // Strong fires get two smaller side flames behind the main one.
    if (strength > 0.45f) {
        flame(base.x - 6.0f, base.y - 3.0f, height * 0.62f, !flip);
        flame(base.x + 6.0f, base.y - 4.0f, height * 0.62f, flip);
    }
    flame(base.x, base.y - 1.0f, height, flip);
}

} // namespace game
