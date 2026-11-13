#pragma once

#include <engine/entity/Entity.h>
#include <engine/math/Rect.h>
#include <engine/physics/Collision.h>

namespace engine {
class Texture;
}

namespace game {

// Textures a Fire draws with. Owned by the gameplay scene; must outlive the fires.
struct FireArt {
    const engine::Texture* atlas = nullptr; // placeholder atlas (for the Glow sprite)
    const engine::Texture* flame = nullptr; // assets/items/fire.png
    engine::Rect flameSource;               // visible part of the flame image
};

// A fire burning on one tree tile. Its intensity works like HP: water lowers it, and
// at zero the fire is out. A fire that has not been hit for a while grows back toward
// full strength, so the player has to keep spraying instead of tapping once.
// The fire also keeps two clocks that FireSystem acts on: how long the tree has burnt
// (long enough -> the tree becomes sawit) and when it may spread to another tree.
class Fire : public engine::Entity {
public:
    Fire(int tileX, int tileY, const FireArt& art);

    void Update(float dt) override;
    void Draw(engine::SpriteBatch& batch, float alpha) const override;

    // Lowers the intensity and pauses regrowth for a moment.
    void ApplyWater(float amount);

    float GetIntensity() const { return m_intensity; }
    float GetIntensityFraction() const;
    bool IsExtinguished() const { return m_intensity <= 0.0f; }

    // Seconds of burning, counted faster for strong fires (half speed at zero intensity).
    float GetBurnTime() const { return m_burnTime; }
    // A strong fire (see SpreadMinIntensityFraction) counts down to its next spread;
    // a weak one is held back. FireSystem calls RestartSpreadTimer after spreading.
    bool IsReadyToSpread() const { return m_spreadTimer <= 0.0f; }
    void RestartSpreadTimer();

    // Water must touch this circle to count as a hit; it shrinks as the fire weakens.
    engine::Circle GetHitArea() const;
    int GetTileX() const { return m_tileX; }
    int GetTileY() const { return m_tileY; }
    // World y of the burning tree's roots, used to sort the fire between trees and the player.
    float GetBaseY() const { return m_position.y; }

private:
    FireArt m_art;
    int m_tileX;
    int m_tileY;
    float m_intensity;
    float m_sinceWater = 999.0f; // seconds since the last water hit
    float m_burnTime = 0.0f;
    float m_spreadTimer;         // seconds until the next spread (only counts while strong)
    float m_time;                // animation clock, offset per fire so they do not flicker in sync
};

} // namespace game
