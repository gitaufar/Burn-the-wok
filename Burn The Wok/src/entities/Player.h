#pragma once

#include "GameConfig.h"
#include "systems/WaterTank.h"

#include <engine/graphics/Texture.h>
#include <engine/math/Rect.h>
#include <engine/math/Vec2.h>

#include <array>
#include <memory>

namespace engine {
class Input;
class SpriteBatch;
class Texture;
} // namespace engine

namespace game {

class ForestMap;

enum class Facing { Down, Up, Left, Right };

// The forest ranger controlled by the player: walks in 4 directions, cannot pass
// through solid map tiles (trees, water) and carries the water tank. Standing at the
// edge of a pond refills the tank; the spraying itself is done by Sprayer.
class Player {
public:
    explicit Player(engine::Vec2 position) : m_position(position), m_previousPosition(position) {}
    // Call after the graphics context is ready.
    void LoadArtwork();

    // Moves with WASD/arrows; trees and water stop the player, who slides along them.
    // Also refills the tank while the player stands next to water.
    void Update(const engine::Input& input, float dt, const ForestMap& map);

    // Turns the ranger toward a (normalized) direction and raises the spray gun: until
    // the next Update() the "holding the gun" sprites are drawn instead of the walking
    // ones. Call it every update while the trigger is held. Overrides the facing from
    // movement for this update.
    void Aim(engine::Vec2 direction);
    // World point where water leaves the spray gun for a given aim direction: the
    // nozzle of the gun as drawn in the sprite for the facing that aim produces.
    engine::Vec2 GetNozzlePosition(engine::Vec2 aim) const;

    WaterTank& GetTank() { return m_tank; }
    const WaterTank& GetTank() const { return m_tank; }
    // True while the tank is being filled (standing at a water source, tank not full).
    bool IsRefilling() const { return m_refilling; }
    // True while the ranger actually moves (not while pushing against a wall).
    bool IsMoving() const { return m_moving; }

    // Collision box in world pixels. Only the feet collide (not the whole 16x16 sprite),
    // so the ranger's head can overlap a tree above them, which looks natural top-down.
    engine::Rect GetHitbox() const;
    engine::Vec2 GetPosition() const { return m_position; }

    // Position between the previous and the current update, rounded to whole world
    // pixels so the sprite lines up with the tile grid.
    engine::Vec2 GetRenderPosition(float alpha) const;
    float GetRenderFeetY(float alpha) const {
        return GetRenderPosition(alpha).y + HitboxOffsetY + HitboxHeight;
    }

    void Draw(engine::SpriteBatch& batch, const engine::Texture& atlas, float alpha, float zoom) const;

    Facing GetFacing() const { return m_facing; }

private:
    // [0..15] walking (tank on the back), [16..31] spraying (gun in the hands);
    // inside each set: Facing * 4 + walk frame.
    static constexpr int FramesPerSet = 16;
    std::array<std::unique_ptr<engine::Texture>, FramesPerSet * 2> m_frames;
    std::array<engine::Rect, FramesPerSet * 2> m_sources{};
    float m_artScale = 1.0f;
    void UpdateFacing(engine::Vec2 direction);
    static Facing FacingFor(engine::Vec2 direction); // dominant axis of direction

    // Hitbox size and placement relative to m_position (the sprite center).
    static constexpr float HitboxWidth = 10.0f;
    static constexpr float HitboxHeight = 6.0f;
    static constexpr float HitboxOffsetY = 2.0f; // top of the hitbox, below the center

    engine::Vec2 m_position;         // center of the sprite, world pixels
    engine::Vec2 m_previousPosition; // position at the previous update (for interpolation)
    Facing m_facing = Facing::Down;
    float m_walkTime = 0.0f;
    bool m_moving = false;
    bool m_aiming = false; // Aim() was called since the last Update()
    WaterTank m_tank{config::TankCapacity};
    bool m_refilling = false;
};

} // namespace game
