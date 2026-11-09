#pragma once

#include <engine/math/Rect.h>
#include <engine/math/Vec2.h>

namespace engine {
class Input;
class SpriteBatch;
class Texture;
} // namespace engine

namespace game {

class ForestMap;

enum class Facing { Down, Up, Left, Right };

// The forest ranger controlled by the player: walks in 4 directions and cannot pass
// through solid map tiles (trees, water). The spray / water tank arrive later in Stage 2.
class Player {
public:
    explicit Player(engine::Vec2 position) : m_position(position), m_previousPosition(position) {}

    // Moves with WASD/arrows; trees and water stop the player, who slides along them.
    void Update(const engine::Input& input, float dt, const ForestMap& map);

    // Collision box in world pixels. Only the feet collide (not the whole 16x16 sprite),
    // so the ranger's head can overlap a tree above them, which looks natural top-down.
    engine::Rect GetHitbox() const;
    engine::Vec2 GetPosition() const { return m_position; }

    // Position between the previous and the current update, rounded to whole world
    // pixels so the sprite lines up with the tile grid.
    engine::Vec2 GetRenderPosition(float alpha) const;

    void Draw(engine::SpriteBatch& batch, const engine::Texture& atlas, float alpha) const;

    Facing GetFacing() const { return m_facing; }

private:
    void UpdateFacing(engine::Vec2 direction);

    // Hitbox size and placement relative to m_position (the sprite center).
    static constexpr float HitboxWidth = 10.0f;
    static constexpr float HitboxHeight = 6.0f;
    static constexpr float HitboxOffsetY = 2.0f; // top of the hitbox, below the center

    engine::Vec2 m_position;         // center of the sprite, world pixels
    engine::Vec2 m_previousPosition; // position at the previous update (for interpolation)
    Facing m_facing = Facing::Down;
    float m_walkTime = 0.0f;
    bool m_moving = false;
};

} // namespace game
