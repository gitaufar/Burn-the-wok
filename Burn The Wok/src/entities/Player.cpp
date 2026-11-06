#include "entities/Player.h"

#include "GameConfig.h"
#include "art/PlaceholderArt.h"

#include <engine/graphics/SpriteBatch.h>
#include <engine/graphics/Texture.h>
#include <engine/input/Input.h>

#include <algorithm>

namespace game {

using engine::Key;
using engine::Vec2;

void Player::Update(const engine::Input& input, float dt, const engine::Rect& walkableArea) {
    m_previousPosition = m_position;

    Vec2 direction{};
    if (input.IsKeyDown(Key::A) || input.IsKeyDown(Key::Left)) direction.x -= 1.0f;
    if (input.IsKeyDown(Key::D) || input.IsKeyDown(Key::Right)) direction.x += 1.0f;
    if (input.IsKeyDown(Key::W) || input.IsKeyDown(Key::Up)) direction.y -= 1.0f;
    if (input.IsKeyDown(Key::S) || input.IsKeyDown(Key::Down)) direction.y += 1.0f;

    if (direction != Vec2{}) {
        UpdateFacing(direction);
        // Normalize so diagonal movement is not ~41% faster than straight movement.
        m_position += direction.Normalized() * config::PlayerSpeed * dt;
    }

    const float half = config::TileSize * 0.5f;
    m_position.x = std::clamp(m_position.x, walkableArea.Left() + half, walkableArea.Right() - half);
    m_position.y = std::clamp(m_position.y, walkableArea.Top() + half, walkableArea.Bottom() - half);
}

void Player::UpdateFacing(Vec2 direction) {
    const bool horizontal = direction.x != 0.0f;
    const bool vertical = direction.y != 0.0f;

    // Moving diagonally: keep the current facing if it is one of the two directions,
    // otherwise the sprite would flicker between them.
    if (horizontal && vertical) {
        const bool stillMatches = (m_facing == Facing::Left && direction.x < 0.0f) ||
                                  (m_facing == Facing::Right && direction.x > 0.0f) ||
                                  (m_facing == Facing::Up && direction.y < 0.0f) ||
                                  (m_facing == Facing::Down && direction.y > 0.0f);
        if (stillMatches) {
            return;
        }
    }

    if (horizontal) {
        m_facing = direction.x < 0.0f ? Facing::Left : Facing::Right;
    } else {
        m_facing = direction.y < 0.0f ? Facing::Up : Facing::Down;
    }
}

Vec2 Player::GetRenderPosition(float alpha) const {
    return engine::Round(engine::Lerp(m_previousPosition, m_position, alpha));
}

void Player::Draw(engine::SpriteBatch& batch, const engine::Texture& atlas, float alpha) const {
    SpriteId sprite = SpriteId::PlayerDown;
    switch (m_facing) {
    case Facing::Down:  sprite = SpriteId::PlayerDown;  break;
    case Facing::Up:    sprite = SpriteId::PlayerUp;    break;
    case Facing::Left:  sprite = SpriteId::PlayerLeft;  break;
    case Facing::Right: sprite = SpriteId::PlayerRight; break;
    }

    const Vec2 center = GetRenderPosition(alpha);
    const float size = static_cast<float>(config::TileSize);
    batch.Draw(atlas, {center.x - size * 0.5f, center.y - size * 0.5f, size, size},
               GetSpriteSource(sprite));
}

} // namespace game
