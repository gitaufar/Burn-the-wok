#pragma once

#include <engine/math/Rect.h>
#include <engine/math/Vec2.h>

namespace engine {
class Input;
class SpriteBatch;
class Texture;
} // namespace engine

namespace game {

enum class Facing { Down, Up, Left, Right };

// The forest ranger controlled by the player. Stage 1: walks in 4 directions.
// The spray / water tank arrive in Stage 2.
class Player {
public:
    explicit Player(engine::Vec2 position) : m_position(position), m_previousPosition(position) {}

    // Keeps the player's center inside walkableArea.
    void Update(const engine::Input& input, float dt, const engine::Rect& walkableArea);

    // Position between the previous and the current update, rounded to whole world
    // pixels so the sprite lines up with the tile grid.
    engine::Vec2 GetRenderPosition(float alpha) const;

    void Draw(engine::SpriteBatch& batch, const engine::Texture& atlas, float alpha) const;

    Facing GetFacing() const { return m_facing; }

private:
    void UpdateFacing(engine::Vec2 direction);

    engine::Vec2 m_position;         // center of the sprite, world pixels
    engine::Vec2 m_previousPosition; // position at the previous update (for interpolation)
    Facing m_facing = Facing::Down;
    float m_walkTime = 0.0f;
    bool m_moving = false;
};

} // namespace game
